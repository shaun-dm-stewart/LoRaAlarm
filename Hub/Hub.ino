/*
*  Title          :  Base Station
*  Desc           :  This firmware monitors the status of a remote node.
*                 :  If a notification is received, it is signaled via a buzzer on the base station.
*                 :
*                 :
*                 :
*  Author         :  Shaun Stewart
*  Created        :  2025-04-21
*  Version        :  1.0  Integration Test
*                 :  2025-04-21  A.1  alpha test
*                 :  2025-04-24  1.0  made variable naming and function naming more consistent.  End to end testing complete
*
*/

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP32_NOW.h>
//#include <WiFiAP.h>
//#include <WiFiClient.h>
//#include <WiFiGeneric.h>
//#include <WiFiMulti.h>
//#include <WiFiScan.h>
//#include <WiFiServer.h>
//#include <WiFiSTA.h>
//#include <WiFiType.h>
//#include <WiFiUdp.h>
#include <WiFi.h>
#include "LoRaWan_APP.h"

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

// LoRa stuff
#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define RX_TIMEOUT_VALUE                            100      // ms

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define BUFFER_SIZE                                 50 // Define the payload size here
// Sensor GPIO pin assignments
#define BUZZERPIN                                    7

typedef enum
{
    IDLING,
    LOWPOWER,
    STATE_RX,
    STATE_TX
} States_t;

typedef enum
{
    IDLE,
    CLEAR,
    SET
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
} LoRaPacket;

constexpr long watchdogInterval = 120000;  // interval at which to send watchdog signal

static RadioEvents_t RadioEvents;
States_t state;
bool sleepMode = false;
int16_t Rssi, rxSize;


bool alarmActive = false;
JsonDocument inDoc;
JsonDocument outDoc;
LoRaPacket packetData;
LoRaPacket selectedState;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

esp_now_peer_info_t peerInfo;

// Function prototypes
// Radio
void onTxDone(void);
void onTxTimeout(void);
void onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr);
void txPacket(void);
// Operation
void handshake(void);
void OnNowDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
void OnNowDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);

void setup()
{
    debug_begin(9600);      // Start up the serial port if in debug mode

    pinMode(BUZZERPIN, OUTPUT);
    digitalWrite(BUZZERPIN, LOW);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        debugln("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Initialised, register the sent data callback
    // get the status of Transmitted packet
    esp_now_register_send_cb(OnNowDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        debugln("Failed to add peer");
        return;
    }
    // Register the data received callback function
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnNowDataRecv));

    selectedState.nodeAddress = 1;          // This is the address of the remote node you wish to control
    selectedState.relay1Enabled = ACTIVE;
    selectedState.relay2Enabled = ACTIVE;

    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    Rssi = 0;

    RadioEvents.TxDone = onTxDone;
    RadioEvents.TxTimeout = onTxTimeout;
    RadioEvents.RxDone = onRxDone;
	RadioEvents.RxTimeout = onRxTimeout;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
        LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
        0, true, 0, 0, LORA_IQ_INVERSION_ON, false);
    state = IDLING;
}

void loop()
{
    switch (state)
    {
        case IDLING:
			handshake();
			break;
        case STATE_TX:
            txPacket();
            break;
        case STATE_RX:
            debugln("into RX mode");
            Radio.Rx(RX_TIMEOUT_VALUE);
            state = LOWPOWER;
            break;
        case LOWPOWER:
            Radio.IrqProcess();
            break;
        default:
            break;
    }
}

// Callback when data is sent
void OnNowDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) 
{
    //debug("\r\nLast Packet Send Status:\t");
    //debugln(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnNowDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len)
{
    memcpy(&selectedState, incomingData, sizeof(selectedState));
    //debug("Bytes received from UI: ");
    //debugln(len);
    if (selectedState.relay1Enabled != packetData.relay1Enabled || selectedState.relay2Enabled != packetData.relay2Enabled)
    {
        debugln("State chenge requested");
        state = STATE_TX;
    }

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packetData, sizeof(packetData));

#ifdef debug_print    
    //if (result == ESP_OK)
    //{
    //    Serial.println("Sent with success");
    //}
    //else
    //{
    //    Serial.println("Error sending the data");
    //}
#endif

}

void handshake(void)
{
    static unsigned long previousMillis = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= watchdogInterval)
    {
        previousMillis = currentMillis;
		state = STATE_TX;  // Set state to TX to send a watchdog signal
    }
}

void txPacket(void)
{
    char outBuffer[BUFFER_SIZE];
    outDoc["g"] = selectedState.nodeAddress;
    outDoc["m"] = selectedState.alarmState;
    outDoc["r1"] = selectedState.relay1Enabled;
    outDoc["r2"] = selectedState.relay2Enabled;
    serializeJson(outDoc, outBuffer);
    debug("Transmitting via radio: ");
    debugln(outBuffer);
    Radio.Send((uint8_t*)outBuffer, strlen(outBuffer));
    state = LOWPOWER;
}

void onTxDone(void)
{
    debugln("TX done...");
    state = STATE_RX;
}

void onTxTimeout(void)
{
    debugln("TX timeout...");
    Radio.Sleep();
    state = STATE_TX;
}

void onRxTimeout(void)
{
    debugln("RX timeout...");
    Radio.Sleep();
    state = STATE_TX;
}

void onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
    static LoRaPacket oBuffer;
    bool updateDisplay = false;
    static char rxpacket[BUFFER_SIZE];
    Rssi = rssi;
    rxSize = size;
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    Radio.Sleep();

    DeserializationError error = deserializeJson(inDoc, rxpacket);

    if (error) {
        debugln(F("deserializeJson() failed: "));
        debugln(error.c_str());
    }
    else
    {
        // Extract the values
        packetData.nodeAddress = inDoc["g"];
        packetData.alarmState = static_cast<DeviceStates_t>(inDoc["m"]);
        packetData.relay1Enabled = static_cast<RelayStates_t>(inDoc["r1"]);
        packetData.relay2Enabled = static_cast<RelayStates_t>(inDoc["r2"]);

        switch (packetData.alarmState)
        {
        case SET:
            debugln("Alarm on");
            digitalWrite(BUZZERPIN, HIGH);
            break;
        case CLEAR:
            debugln("Alarm off");
            digitalWrite(BUZZERPIN, LOW);
        case IDLE:
        default:
            break;
        }
    }

#ifdef debug_print
    Serial.printf("\r\nReceived packet \"%s\" with Rssi %d , length %d\r\n", rxpacket, Rssi, rxSize);
#endif

    state = IDLING;
}