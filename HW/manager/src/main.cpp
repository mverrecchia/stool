// hardware/manager/src/main.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include "HW_NeonManager.h"
#include "DeviceAddresses.h"
#include "TaskManager.h"

#include "HW_config.h"

HW_NeonManager* pManager = nullptr;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    pManager->handleMqttMessage(topic, payload, length);
}

void setup()
{   
    Serial.begin(SERIAL_BAUD);

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(MQTT_BUFFER_SIZE);

    delay(2000);

    pManager = new HW_NeonManager(mqttClient);
    if (!pManager->initialize())
    {
        if (Serial) Serial.println("Failed to initialize HW Manager");
    }

    TaskManager::createManagerUpdateTask(pManager);
}

void loop(){}