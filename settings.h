#include "IPAddress.h"

// Wifi settings
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// MQTT settings
const IPAddress MQTT_HOST_IP = IPAddress(192, 168, 0, 0);
const char* MQTT_HOST_USER = "";
const char* MQTT_HOST_PASSWORD = "";
const char* MQTT_TOPIC_ROOT = "huisautomatie";
const char* MQTT_TOPIC_CONNECT = "/debug/node_connect";
const char* MQTT_TOPIC_ERROR = "/debug/node_error";
const char* MQTT_CLIENT_NAME = "ESP32 Meterkast";
const unsigned short MQTT_TIMEOUT_SECONDS = 15;
const unsigned short MQTT_TIMOUT_MILLIS = MQTT_TIMEOUT_SECONDS * 1000;
const unsigned short MQTT_MAX_LENGTH = 1024;

// OTA settings
const char* OTA_NAME = "";
const char* OTA_PASSWORD = "";

// Data retrieval settings
const unsigned short METER_INTERVAL_SECONDS = 30;
const unsigned short METER_INTERVAL_MILLIS = METER_INTERVAL_SECONDS * 1000;
const unsigned short METER_TIMEOUT_SECONDS = 7;
const unsigned short METER_TIMEOUT_MILLIS = METER_TIMEOUT_SECONDS * 1000;
const unsigned short METER_DATA_MAX_LENGTH = 2048;
const unsigned short METER_CRC_LENGTH = 5;

// Pin definitions
#define PIN_RX 16
#define PIN_RTS 17
#define PIN_LED_READING 2

// Struct linking an OBIS reference to where its value should be published to
typedef struct {
    char reference[12];
    char path[44];
} MqttPath;

// OBIS reference to MQTT publishing path lookup table
const MqttPath MQTT_PATHS[] = {
    { "0-0:1.0.0",    "/meterkast/datetime" },
    { "1-0:1.8.1",    "/meterkast/stroom/geleverd/laagtarief" },
    { "1-0:1.8.2",    "/meterkast/stroom/geleverd/hoogtarief" },
    { "1-0:2.8.1",    "/meterkast/stroom/teruggeleverd/laagtarief" },
    { "1-0:2.8.2",    "/meterkast/stroom/teruggeleverd/hoogtarief" },
    { "0-0:96.14.0",  "/meterkast/stroom/tarief" },
    { "1-0:1.7.0",    "/meterkast/stroom/levering" },
    { "1-0:2.7.0",    "/meterkast/stroom/teruglevering" },
    { "1-0:31.7.0",   "/meterkast/stroom/stroomsterkte" },
    { "0-1:24.2.1",   "/meterkast/gas/geleverd" },
    { "0-0:96.7.21",  "/meterkast/events/storingen" },
    { "0-0:96.7.9",   "/meterkast/events/storingen_lang" },
    { "1-0:99.97.0",  "/meterkast/events/storingen_details" },
    { "1-0:32.32.0",  "/meterkast/events/spanningsdalen" },
    { "1-0:32.36.0",  "/meterkast/events/spanningspieken" },
    { "0-0:96.13.0",  "/meterkast/events/code" },
    { "0-0:96.13.1",  "/meterkast/events/bericht" }
};
