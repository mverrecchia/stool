// HW/include/HW_NeonManager.h
#pragma once

#include "NeonManager.h"
#include "HW_FFTAudioAnalyzer.h"
#include "HW_PlatformUtils.h"
#include "PlatformTypes.h"
#include "MicrophoneSensor.h"
#include "DistanceSensor.h"
#include "driver/rmt.h"
#include "Filter.hpp"

#include <PubSubClient.h>
#include <esp_now.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Adafruit_MCP4728.h>
#include <Preferences.h>

class HW_NeonManager : public NeonManager {
public:
    HW_NeonManager(PubSubClient& mqttClient)
        : NeonManager(new HW_PlatformUtils())
        , m_mqttClient(mqttClient)
    {
        m_managerStatusUpdateTimer.start();
        m_i2cWriteTimer.start();
        m_pFftAnalyzer = new HW_FFTAudioAnalyzer(m_pUtils);
    }
    bool initialize() override;
    void update(float deltaTime) override;

    SupplyOutputType_E getSupplyOutputType(size_t idx) const { return m_supplyOutputType[idx]; }

    void handleMqttMessage(char* topic, byte* payload, unsigned int length);

protected:
    void updateDistanceValue(void) override;

    // Brightness control functions
    virtual void applySupplyBrightness(size_t idx);
    virtual void applySupplyEnable(size_t idx);

private:
    uint8_t m_managerMac[MAC_ADDRESS_SIZE];
    bool m_managerRegistered = false;
    bool m_pwmState = false;
    int64_t m_pwmNextToggleTime = 0;

    Adafruit_MCP4728 m_dac;
    DistanceSensor m_distanceSensor;

    esp_now_peer_info_t m_broadcastPeer;
    BroadcastType_E m_broadcastType;
    Preferences m_nonVolatileStorage;
    SupplyOutputType_E m_supplyOutputType[NUM_SUPPLIES_PER_CONTROLLER];

    PubSubClient& m_mqttClient;
    Timer m_managerStatusUpdateTimer;
    Timer m_i2cWriteTimer;

    TwoWire Wire2{1};

    char m_managerStatusMessage[2048];
    
    StaticJsonDocument<2048> m_managerStatusBuffer;
    StaticJsonDocument<1024> m_profileConfigBuffer;
    StaticJsonDocument<2048> m_audioConfigBuffer;
    StaticJsonDocument<2048> m_manualConfigBuffer;


    bool parseProfileConfig(JsonDocument& doc);
    bool parseAudioConfig(JsonDocument& doc);
    bool parseManualConfig(JsonDocument& doc, size_t idx);
    void broadcastStateToServer(float deltaTime);

    void reconnectMQTT(void);
    void publishManagerStatus(void);

    void saveAudioConfigToEEPROM(AudioConfiguration_S config);
    void loadNonVolatileSettings(void);

    void initializeRmtPwm(void);
    void initializeI2C(void);
    void initializeSupplies(void);

    void applyNeonSettings(void) override;
    void updateRmtPwmDutyCycle(float brightness);
};
