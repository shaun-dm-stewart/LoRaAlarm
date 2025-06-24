/*
*  Title          :  RemoteNode
*  Desc           :  This firmware monitors the status of a PIR sensor.
*                 :  If movement is detected, a message to that effect is trnasmitted to the
*                 :  monitoring base station.  Two relays are made available to enable
*                 :  other external devices if required.
*                 :
*  Author         :  Shaun Stewart
*  Date           :  2025-04-23
*  Version        :  1.1
*  History        : A.0 2025-04-21 Creation
*                 : 1.0 2025-04-23 Integration Test complete
*                 : 1.1 2025-04-24 Watchdog function and sensor scan consolidated and moved
*                 :     relay activation out of the timed loop.  Activation now instant(ish)
*
*/

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include "LoRaWan_APP.h"

// debug stuff
//#define debug_print  // manages most of the print and println debug, not all but most

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

#define TX_OUTPUT_POWER                             14        // dBm

#define RX_TIMEOUT_VALUE                            1000      // ms

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
#define SENSORPIN                                   7
#define RELAYPIN1                                   6
#define RELAYPIN2                                   5

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
} LoRaPacket;

constexpr long watchdogInterval = 5000;   // interval at which to scan alarm sensor
constexpr uint32_t settlingTime = 60000;  // Time to allow the PIR sensor to stabilise

/******************************************************************************************
SET THE NODE ADDRESS BEFORE COMPILING */
constexpr unsigned short thisNodeAddress = 1;
/*******************************************************************************************/

LoRaPacket packetData;
static RadioEvents_t RadioEvents;
States_t state;
int16_t Rssi, rxSize;
JsonDocument inDoc;
JsonDocument outDoc;
DeviceStates_t alarmState = IDLE;

// Function prototypes
void onTxDone(void);
void onTxTimeout(void);
void onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr);
void txPacket(DeviceStates_t msg);
void sensorScanner(void);

void setup()
{
    debug_begin(9600);      // Start up the serial port if in debug mode

    pinMode(SENSORPIN, INPUT_PULLDOWN);
    pinMode(RELAYPIN1, OUTPUT);
    pinMode(RELAYPIN2, OUTPUT);
    digitalWrite(RELAYPIN1, LOW);
    digitalWrite(RELAYPIN2, LOW);

    packetData.relay1Enabled = ACTIVE;
    packetData.relay2Enabled = ACTIVE;

    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    Rssi = 0;

    RadioEvents.TxDone = onTxDone;
    RadioEvents.TxTimeout = onTxTimeout;
    RadioEvents.RxDone = onRxDone;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
        LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
        0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    state = STATE_TX;

    //delay(settlingTime);   // Allow the PIR sensor to stabilise before proceeding
}

void loop()
{
    sensorScanner();
    switch (state)
    {
    case STATE_TX:
        txPacket(alarmState);
        break;
    case STATE_RX:
        debugln("into RX mode");
        Radio.Rx(0);
        state = LOWPOWER;
        break;
    case LOWPOWER:
        Radio.IrqProcess();
        break;
    default:
        break;
    }
}

void sensorScanner(void)
{
    static bool alarmActive = false;
    bool trigger;
    
    if (packetData.alarmState == TEST)
    {
        trigger = true;
	}
    else
    {
        // Read the PIR sensor state
        trigger = (digitalRead(SENSORPIN) == HIGH) ? true : false;
	}

    if (trigger == true)
    {
        if (alarmActive == false)
        {
            alarmActive = true;
            alarmState = SET;
            debugln("Alarm activated");
            if (packetData.relay1Enabled == ACTIVE)
            {
                debugln("Relay 1 set");
                digitalWrite(RELAYPIN1, HIGH);
            }
            if (packetData.relay2Enabled == ACTIVE)
            {
                debugln("Relay 2 set");
                digitalWrite(RELAYPIN2, HIGH);
            }
        }
    }
    else
    {
        if (alarmActive == true)
        {
            alarmActive = false;
            alarmState = CLEAR;
            debugln("Alarm reset, relays cleared");
            digitalWrite(RELAYPIN1, LOW);
            digitalWrite(RELAYPIN2, LOW);
        }
    }
}

void onTxDone(void)
{
    debugln("TX done ...");
    state = STATE_RX;
}

void onTxTimeout(void)
{
    debugln("TX timeout ...");
    Radio.Sleep();
    state = STATE_TX;
}

void txPacket(DeviceStates_t msg)
{
    char outBuffer[BUFFER_SIZE];
    outDoc["g"] = thisNodeAddress;
    outDoc["m"] = msg;
    outDoc["r1"] = packetData.relay1Enabled;
    outDoc["r2"] = packetData.relay2Enabled;
    serializeJson(outDoc, outBuffer);
    debug("Transmitting: ");
    debugln(outBuffer);
    Radio.Send((uint8_t*)outBuffer, strlen(outBuffer));
    state = LOWPOWER;
}

void onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
    static char rxPacket[BUFFER_SIZE];
    Rssi = rssi;
    rxSize = size;
    memcpy(rxPacket, payload, size);
    rxPacket[size] = '\0';
    Radio.Sleep();

    DeserializationError error = deserializeJson(inDoc, rxPacket);

    if (error) {
        debugln(F("deserializeJson() failed: "));
        debugln(error.c_str());
    }
    else
    {
        // Extract the values
        packetData.nodeAddress = inDoc["g"];
        if (packetData.nodeAddress == thisNodeAddress)
        {
            packetData.alarmState = static_cast<DeviceStates_t>(inDoc["m"]);
            packetData.relay1Enabled = static_cast<RelayStates_t>(inDoc["r1"]);
            packetData.relay2Enabled = static_cast<RelayStates_t>(inDoc["r2"]);
            debug("Relay 1 state: ");
            debug(packetData.relay1Enabled);
            debug(", Relay 2 state: ");
            debugln(packetData.relay2Enabled);
        }
        else
        {
            debugln("Not for this node");
        }
    }

#ifdef debug_print
    Serial.printf("\r\nReceived packet \"%s\" with Rssi %d , length %d\r\n", rxPacket, Rssi, rxSize);
    Serial.println("Waiting to send next packet");
#endif

    state = STATE_TX;
}