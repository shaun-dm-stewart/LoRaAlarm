/*
*  Title          :  AlarmConsole
*  Desc           :  This firmware provides the UI.
*                 :  If a notification is received, it is signaled via the UI.
*                 :  The Console is able to enable and disable the relay outputs on the remote node
*                 :  via the base station
*                 :
*  Author         :  Shaun Stewart
*  Created        :  2025-04-21
*  Version        :  1.0  Integration Test
*                 :  2025-04-25  A.1  alpha test
*                 :  2025-04-26  1.0  made variable naming and function naming more consistent.  End to end testing complete
*
*/

#include <esp_now.h>
#include <WiFi.h>
#include <lvgl.h>
#include "ui.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "actions.h"
//#include "D:/Projects/Arduino/libraries/lvgl/src/display/lv_display_private.h"

// debug stuff
//#define debug_print  // manages most of the print and println debug

#if defined debug_print
#define debug_begin(x)        Serial.begin(x)
#define debug(x)              Serial.print(x)
#define debugln(x)            Serial.println(x)
#else
#define debug_begin(x)
#define debug(x)
#define debugln(x)
#endif

#define BUFFER_SIZE 50 // Define the payload size here

// ----------------------------
// Touch Screen pins
// ----------------------------
// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
#define TFT_BACK_LIGHT_PIN 21

#define TFT_BACKLIGHT_CHANNEL 0
#define TFT_BACKLIGHT_FREQUENCY 12000
#define TFT_BACKLIGHT_RESOLUTION_BITS 8

constexpr long watchdogInterval = 500;  // interval at which to send watchdog signal
constexpr long saverInterval = 120000; // interval to switch to saver screen
uint32_t saverMillis = 0;

SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

/*Set to your screen resolution*/
#define TFT_HOR_RES 240
#define TFT_VER_RES 320

/* lvgl declarations*/
lv_indev_t* indev;      //Touchscreen input device
uint8_t* draw_buf;      //draw_buf is allocated on heap otherwise the static area is too big on ESP32 at compile

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

/*
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif
*/

void initialiseEspNow(void);
void initialiseGUI(void);
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);
void UpdateDisplay(void);
void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);
void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data);
void turn_backlight_off(void);
void turn_backlight_on(void);

lv_display_t* disp;

typedef enum
{
    IDLE,
    CLEAR,
    SET,
    TEST
} DeviceStates_t;

typedef enum
{
    INACTIVE,
    ACTIVE
} RelayStates_t;

typedef struct
{
    unsigned short nodeAddress;
    DeviceStates_t alarmState;
    RelayStates_t relay1Enabled;
    RelayStates_t relay2Enabled;
    unsigned long rxTimeoutCount;
    int16_t signalStrength;
} LoRaPacket;

LoRaPacket txBuffer;
LoRaPacket selectedState;
LoRaPacket incomingPacket;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
esp_now_peer_info_t peerInfo;
bool espNowBusy = false;
unsigned short selectedNode = 1;    // Hard code 1 for now but we may want to increase the node count in the future
bool saverActive = false; // Used to track if the saver screen is active
ScreensEnum screenID = SCREEN_ID_MAIN;

void processScreenRequest()
{
    saverActive = false; // Reset saver active state
    saverMillis = millis(); // Reset saver timer
    loadScreen(screenID);
}

extern "C" void action_load_last(lv_event_t* e) 
{
	processScreenRequest(); // Load the last screen requested
    debugln("action_load_last_page");
}

extern "C" void action_load_settings(lv_event_t* e)
{
    screenID = SCREEN_ID_SETTINGS; // Set the screen ID to settings
    processScreenRequest();
    debugln("action_load_settings");
}

extern "C" void action_load_main(lv_event_t* e)
{
	screenID = SCREEN_ID_MAIN; // Set the screen ID to main
    processScreenRequest();
    debugln("action_load_main");
}

extern "C" void action_load_stats(lv_event_t* e) 
{
	screenID = SCREEN_ID_STATS; // Set the screen ID to stats
    processScreenRequest();
    debugln("action_load_stats");
}

extern "C" void action_sw_state_changed(lv_event_t* e)
{
    debugln("action_sw_state_changed");
    lv_event_code_t code = lv_event_get_code(e);              //Get the event code
    lv_obj_t* swObj = (lv_obj_t*)lv_event_get_target(e);     //Switch that generated the event

    //uint64_t data = (uint64_t)lv_event_get_user_data(e);
    //debug("Switch: ");
    //debugln(data);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        bool isChecked = lv_obj_has_state(swObj, LV_STATE_CHECKED);  //Returns true/false
        if (swObj == objects.sw_relay1)
        {
            if (isChecked)
            {
                Serial.println("Relay 1 enabled");
                txBuffer.relay1Enabled = ACTIVE;
            }
            else
            {
                Serial.println("Relay 1 disabled");
                txBuffer.relay1Enabled = INACTIVE;
            }
        }
		else if (swObj == objects.sw_relay2)
        {
            if (isChecked)
            {
                debugln("Relay 2 enabled");
                txBuffer.relay2Enabled = ACTIVE;
            }
            else
            {
                Serial.println("Relay 2 disabled");
                txBuffer.relay2Enabled = INACTIVE;
            }
        }
        else
        { 
            if (isChecked)
            {
                debugln("Alarm state: TEST");
                txBuffer.alarmState = TEST;
            }
            else
            {
                debugln("ALARM State: NOMINAL");
                txBuffer.alarmState = CLEAR;
            }
        }
    }
}

extern "C" void action_send_states(lv_event_t* e)
{
    debugln("action_send_states");
    // Write display buffer to selectedState
    selectedState.nodeAddress = selectedNode;
    selectedState.relay1Enabled = txBuffer.relay1Enabled;
    selectedState.relay2Enabled = txBuffer.relay2Enabled;
	selectedState.alarmState = txBuffer.alarmState;
}

extern "C" void action_show_backlight(lv_event_t* e)
{
    // TODO: Implement action show_backlight here
}

void loadScreenSaver()
{
    if(!saverActive)
    {
		saverActive = true;
        loadScreen(SCREEN_ID_SAVER);
    }
}

void sendData()
{
    debugln("sendData");
    espNowBusy = true;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&selectedState, sizeof(selectedState));
    if (result == ESP_OK)
    {
        debug("sendData() Transmitting: ");
        debug("Node address: ");
        debug(selectedState.nodeAddress);
        debug(", Alarm state: ");
        debug(selectedState.alarmState);
        debug(", Relay 1 enabled: ");
        debug(selectedState.relay1Enabled);
        debug(", Relay 2 enabled: ");
        debugln(selectedState.relay2Enabled);
    }
    else
    {
        debugln("Error sending the data");
    }
}

void setup()
{
    // Initialise Serial Monitor
    debug_begin(115200);
    pinMode(TFT_BACK_LIGHT_PIN, OUTPUT);
    ledcAttachChannel(TFT_BACK_LIGHT_PIN, TFT_BACKLIGHT_FREQUENCY, TFT_BACKLIGHT_RESOLUTION_BITS, TFT_BACKLIGHT_CHANNEL);
    initialiseEspNow();
    initialiseGUI();
}

void loop()
{
    static uint32_t lastTick = 0;  //Used to track the tick timer
    static uint32_t previousMillis = 0;
    uint32_t currentMillis = millis();

    if (currentMillis - previousMillis >= watchdogInterval)
    {
        // Disable the display updates until ESP Now has completed a round trip
        // LVGL is not thread safe
        previousMillis = currentMillis;
        sendData();
    }

    if(currentMillis - saverMillis >= saverInterval)
    {
        saverMillis = currentMillis;
        loadScreenSaver(); // Load the saver screen after 60 seconds of inactivity
	}

    lv_tick_inc(millis() - lastTick);  //Update the tick timer. Tick is new for LVGL 9

    if (!espNowBusy)
    {
        lastTick = millis();
        lv_timer_handler();  //Update the UI
    }
    delay(5);
}

void initialiseEspNow()
{
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        debugln("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Transmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        debugln("Failed to add peer");
        return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    debugln("ESP Now setup done");
}

void initialiseGUI()
{
    //Initialise the touchscreen
    touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
    touchscreen.begin(touchscreenSpi);                                         /* Touchscreen init */
    touchscreen.setRotation(2);                                                /* Inverted landscape orientation to match screen */

    //Initialise LVGL GUI
    lv_init();

    draw_buf = new uint8_t[DRAW_BUF_SIZE];
    //lv_display_t *disp;
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

    //Initialise the XPT2046 input device driver
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    //Integrate EEZ Studio GUI
    ui_init();

    debugln("LVGL setup done");
}

// Callback when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
    debug("onDataSent: ");
    debugln(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len)
{
    memcpy(&incomingPacket, incomingData, sizeof(incomingPacket));

    debug("onDataRecv: ");
    debug("Node address: ");
    debug(incomingPacket.nodeAddress);
    debug(", Alarm state: ");
    debug(incomingPacket.alarmState);
    debug(", Relay 1 enabled: ");
    debug(incomingPacket.relay1Enabled);
    debug(", Relay 2 enabled: ");
    debugln(incomingPacket.relay2Enabled);

    UpdateDisplay();
}

void UpdateDisplay()
{
    debugln("updateDisplay");

	static bool alarmTriggered = false;
	static uint32_t triggers = 0;   
    static bool ledOn = true;
    char tempBuffer[BUFFER_SIZE];

    if (ledOn)
    {
        lv_led_set_color(objects.led_watchdog, lv_color_hex(0xff00ff00));
        ledOn = false;
    }
    else
    {
        lv_led_set_color(objects.led_watchdog, lv_color_hex(0xff000000));
        ledOn = true;
    }

    switch (incomingPacket.alarmState)
    {
    case SET:
        if (!alarmTriggered)
        {
            triggers++;
            alarmTriggered = true;
			saverActive = false; // Reset saver active state
			screenID = SCREEN_ID_MAIN; // Set the screen ID to main
            loadScreen(screenID);
            debug("Alarm triggered: ");
            debugln(triggers);
		}
        lv_led_set_color(objects.led_state, lv_color_hex(0xffff0000));
        break;
    case CLEAR:
    case IDLE:
        lv_led_set_color(objects.led_state, lv_color_hex(0xff00ff00));
        if (alarmTriggered)
        {
            debug("Alarm cleared: ");
            debugln(triggers);
            alarmTriggered = false;
		}
        break;
    default:
        lv_led_set_color(objects.led_state, lv_color_hex(0xff0000ff));
        break;
    }

    switch (incomingPacket.relay1Enabled)
    {
    case ACTIVE:
        lv_led_set_color(objects.relay1_state_led, lv_color_hex(0xff00ff00));
        lv_label_set_text(objects.relay1_state, "Enabled");
        break;
    default:
        lv_led_set_color(objects.relay1_state_led, lv_color_hex(0xffff0000));
        lv_label_set_text(objects.relay1_state, "Disabled");
        break;
    }

    switch (incomingPacket.relay2Enabled)
    {
    case ACTIVE:
        lv_led_set_color(objects.relay2_state_led, lv_color_hex(0xff00ff00));
        lv_label_set_text(objects.relay2_state, "Enabled");
        break;
    default:
        lv_led_set_color(objects.relay2_state_led, lv_color_hex(0xffff0000));
        lv_label_set_text(objects.relay2_state, "Disabled");
        break;
    }

    sprintf(tempBuffer, "%u", incomingPacket.nodeAddress);
    lv_label_set_text(objects.lbl_node_id, tempBuffer);
    lv_label_set_text(objects.lbl_node_id_1, tempBuffer);
    sprintf(tempBuffer, "%lu", incomingPacket.rxTimeoutCount);
    lv_label_set_text(objects.lbl_retry_count, tempBuffer);
    sprintf(tempBuffer, "%d", incomingPacket.signalStrength);
    lv_label_set_text(objects.lbl_rssi, tempBuffer);
    sprintf(tempBuffer, "%lu", triggers);
    lv_label_set_text(objects.lbl_activations, tempBuffer);

    espNowBusy = false;   // We can resume updating the display
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map)
{
    /*Call it to tell LVGL you are ready*/
    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data)
{
    if (touchscreen.touched())
    {
        TS_Point p = touchscreen.getPoint();
        //Some very basic auto calibration so it doesn't go out of range
        if (p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
        if (p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
        if (p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
        if (p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
        //Map this to the pixel position
        data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1, TFT_HOR_RES); /* Touchscreen X calibration */
        data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1, TFT_VER_RES); /* Touchscreen Y calibration */
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void turn_backlight_off(void)
{
    ledcWrite(TFT_BACKLIGHT_CHANNEL, 0); // Turn off PWM
}

void turn_backlight_on(void)
{
    int dutyCycle = (int)round(255 * 1.00); // Example: Set to 75% brightness
    ledcWrite(TFT_BACKLIGHT_CHANNEL, dutyCycle);
}
