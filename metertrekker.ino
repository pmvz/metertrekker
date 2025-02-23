#include <ArduinoOTA.h>
#include <HardwareSerial.h>
#include <MQTT.h>
#include <WiFi.h>
#include "settings.h"


HardwareSerial SerialP1(2);
WiFiClient wifiClient;
MQTTClient mqttClient(MQTT_MAX_LENGTH);

char meterdata[METER_DATA_MAX_LENGTH];
char crc[METER_CRC_LENGTH];
long last_read_time = 0;

void setup_wifi() {
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);  // min tx power
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to network ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.print(" Connected! IP: ");
    Serial.println(WiFi.localIP());
}


void setup_ota()
{
    ArduinoOTA.setHostname(OTA_NAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
}


void connect_mqtt()
{
    Serial.print("(Re)connecting to MQTT broker at ");
    Serial.print(MQTT_HOST_IP);

    while (!mqttClient.connect(MQTT_CLIENT_NAME, MQTT_HOST_USER, MQTT_HOST_PASSWORD))
    {
        Serial.print('.');
        delay(100);
    }

    Serial.println(".. Connected!");
    
    char message[64];
    snprintf(message, 64, "%s (re)connected!", MQTT_CLIENT_NAME);
    mqttClient.publish(MQTT_TOPIC_CONNECT, message);
}


void setup() {
    // Setup serial
    Serial.begin(115200);
    SerialP1.begin(115200, SERIAL_8N1, PIN_RX, -1, true);  // inverted Rx, no Tx
    delay(1000);
    Serial.println();

    // Setup pins
    pinMode(PIN_RTS, OUTPUT);
    pinMode(PIN_LED_READING, OUTPUT);

    // Setup wifi, OTA updates and MQTT
    setup_wifi();
    setup_ota();
    mqttClient.begin(MQTT_HOST_IP, wifiClient);
    mqttClient.setKeepAlive(60);
    connect_mqtt();

    last_read_time = millis();
}


void loop() {
    // Handle OTA and MQTT connectvity
    ArduinoOTA.handle();
    if (!mqttClient.connected())
        connect_mqtt();
    
    mqttClient.loop();

    // Check if it is time to read the data yet
    long current_time = millis();
    if (current_time - last_read_time < METER_INTERVAL_MILLIS)
    {
        delay(100);
        return;
    }
    last_read_time = current_time;

    // Attempt to read data from the meter
    bool succes = try_read_P1();
    if (!succes)
        return;  // loop will restart on timeouts

    Serial.println("\nFull unprocessed data:");
    Serial.println(meterdata);
    Serial.println();

    // TODO: CRC
    process_data();
}


// Try to get a response from the meter P1 port within the timeout frame.
// Returns true if data was retrieved, and false on timeout.
bool try_read_P1()
{
    // Set request pin high and wait until there is data
    Serial.print("Requesting to send data ..");
    digitalWrite(PIN_RTS, HIGH);
    digitalWrite(PIN_LED_READING, HIGH);

    long request_start_time = millis();
    while(!SerialP1.available())
    {
        delay(100);
        Serial.print('.');

        // Handle timeout
        if (millis() - request_start_time >= METER_TIMEOUT_MILLIS)
        {
            Serial.println("\nTimeout while requesting data!");
            digitalWrite(PIN_RTS, LOW);
            digitalWrite(PIN_LED_READING, LOW);
            // TODO: log error to mqtt
            return false;
        }
    }
    
    // Move data into meterdata and crc variables
    memset(meterdata, 0, sizeof(meterdata));
    size_t len = SerialP1.readBytesUntil('!', meterdata, METER_DATA_MAX_LENGTH);
    meterdata[len] = '!';
    SerialP1.readBytes(crc, METER_CRC_LENGTH);

    // Set request pin low
    Serial.println("\nData received");
    digitalWrite(PIN_RTS, LOW);
    digitalWrite(PIN_LED_READING, LOW);

    // Empty UART buffer of any remaining data
    while (SerialP1.available())
        SerialP1.read();

    return true;
}


void process_data()
{
    char* lineptr;
    String line, reference, value, publishPath;

    // Entries are delimited by \r\n, iterate over them with strtok
    lineptr = strtok(meterdata, "\r\n");
    while (lineptr = strtok(NULL, "\r\n"))
    {
        line = String(lineptr);
        if (line.charAt(0) != '0' && line.charAt(0) != '1')
            continue;

        // Get the OBIS reference
        size_t i = line.indexOf('(');
        reference = line.substring(0, i);

        // Get the MQTT publishing path from the reference
        publishPath = MQTT_TOPIC_ROOT;
        for (size_t j = 0; j < sizeof(MQTT_PATHS)/sizeof(MQTT_PATHS[0]); j++)
            if (strcmp(MQTT_PATHS[j].reference, reference.c_str()) == 0)
                publishPath += MQTT_PATHS[j].path;

        // If the reference was not in the array, ignore this data entry
        if (strcmp(publishPath.c_str(), MQTT_TOPIC_ROOT) == 0)
            continue;
        
        // Get the value and publish it to the specified path where it is further processed
        value = line.substring(i);
        mqttClient.publish(publishPath, value);
    }
}











