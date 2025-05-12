// hardware/manager/src/HW_NeonManager.cpp
#include "HW_NeonManager.h"
#include "HW_PlatformUtils.h"
#include "PlatformTypes.h"
#include "PlatformConstants.h"
#include "DistanceSensor.h"
#include "DeviceAddresses.h"
#include "HW_config.h"

#include "driver/rmt.h"
#include <WiFi.h>

const char* MQTT_CLIENT_ID = "stool_manager";
const char* MQTT_MANAGER_TOPIC_PREFIX = "stool/manager/status";
const char* MQTT_PROFILE_TOPIC = "stool/profile";
const char* MQTT_AUDIO_CONFIG_TOPIC = "stool/audio_config";
const char* MQTT_MANUAL_PREFIX = "stool/manual/";

const char* MQTT_LOCK_REQUEST_TOPIC = "stool/lock/request";
const char* MQTT_LOCK_RESPONSE_TOPIC = "stool/lock/response";

bool HW_NeonManager::initialize(void)
{
    bool success = false;

    // TODO: re-enable/re-test EEPROM
    // EEPROM.begin(HW::EEPROM_SIZE);
    // m_nonVolatileStorage.begin(HW::NVS_NAMESPACE, false);
    // loadNonVolatileSettings();

    initializeRmtPwm();
    initializeI2C();
    initializeSupplies();

    success = m_pFftAnalyzer->begin();

    return true;
}

void HW_NeonManager::update(float deltaTime)
{    
    checkLockTimeout();
    updateSensors();
    NeonManager::update(deltaTime);
}

void HW_NeonManager::handleMqtt()
{
    if (!m_mqttClient.connected())
    {
        reconnectMQTT();
    }
    m_mqttClient.loop();
}


void HW_NeonManager::updateSensors()
{
    updateDistanceValue();
    // TODO: still need to implement current sense + feedback
    // updateCurrentSense();
}

bool HW_NeonManager::parseManualConfig(JsonDocument& doc, size_t idx)
{
    m_directControlManual = true;

    for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
    {
        m_manualRequestSupplies[supplyIdx].id = doc["supplies"][supplyIdx]["id"] | 0;
        m_manualRequestSupplies[supplyIdx].enable = doc["supplies"][supplyIdx]["enabled"] | false;
        m_manualRequestSupplies[supplyIdx].brightness = doc["supplies"][supplyIdx]["brightness"] | 0.0f;
    }

    return true;
}

bool HW_NeonManager::parseProfileConfig(JsonDocument& doc)
{
    m_directControlProfile = true;

    for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    {
        // json key/value pairs come in with a string-ified int as the key
        char key[4]; 
        snprintf(key, sizeof(key), "%zu", idx);
        
        if (doc.containsKey(key))
        {
            JsonObject controller = doc[key];
            if (controller)
            {
                uint8_t index = controller["index"] | 0;
                char buffer[300];
                snprintf(buffer, sizeof(buffer), "Controller %d: type %d, magnitude %f, frequency %f, phase %f, enable %d, stopProfile %d",
                        index, controller["profileType"] | 0, controller["magnitude"] | 0.0f, controller["frequency"] | 1.0f,
                        controller["phase"] | 0.0f, controller["enable"] | false, controller["stopProfile"] | false);
                m_pUtils->logWarning(buffer);
                m_profileRequestParameters[index].type = static_cast<ProfileType_E>(controller["profileType"] | 0);
                m_profileRequestParameters[index].magnitude = controller["magnitude"] | 0.0f;
                m_profileRequestParameters[index].frequency = controller["frequency"] | 1.0f;
                m_profileRequestParameters[index].phase = controller["phase"] | 0.0f;
                m_profileRequestParameters[index].enable = controller["enable"] | false;
                m_profileRequestParameters[index].stopProfile = controller["stopProfile"] | false;
            }
        }
    }

    return true;
}

bool HW_NeonManager::parseAudioConfig(JsonDocument& doc)
{
    AudioAssignmentMode_E mode = AudioAssignmentMode_E::FIXED;
    FrequencyBand_E frequencyFlags[NUM_CONTROLLERS][NUM_SUPPLIES_PER_CONTROLLER];
    float magnitudeThresholds[NUM_AUDIO_BUCKETS];
    float lowFrequencyWeights[NUM_LOW_BINS];
    float midFrequencyWeights[NUM_MID_BINS];
    float highFrequencyWeights[NUM_HIGH_BINS];

    // TODO: Add support for different audio modes
    bool allowMultipleActive = doc["audioAllowMultipleActive"] | false;
    // const char* modeStr = doc["audioMode"];
    // if (strcmp(modeStr, "fixed") == 0) 
    // {
    //     mode = AudioAssignmentMode_E::FIXED;
    // }
    // else if (strcmp(modeStr, "random") == 0)
    // {
    //     mode = AudioAssignmentMode_E::RANDOM;
    // }

    // else if (strcmp(modeStr, "sequential") == 0)
    // {
    //     mode = AudioAssignmentMode_E::SEQUENTIAL;
    // }
    // else
    // {
    //     m_pUtils->logError("Invalid audio mode");
    // }

    JsonArray thresholds = doc["audioMagnitudeThresholds"];

    for (size_t idx = 0; idx < NUM_AUDIO_BUCKETS; idx++)
    {
        magnitudeThresholds[idx] = thresholds[idx];
    }

    JsonArray supplyFlags = doc["audioSupplyFlags"];

    for (size_t controllerIdx = 0; controllerIdx < NUM_CONTROLLERS; controllerIdx++)
    {
        for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
        {
            frequencyFlags[controllerIdx][supplyIdx] = supplyFlags[controllerIdx][supplyIdx];
        }
    }

    JsonObject weights = doc["audioWeights"];
    JsonArray lowWeights = weights["low"];
    JsonArray midWeights = weights["mid"];
    JsonArray highWeights = weights["high"];

    if (lowWeights && lowWeights.size() == NUM_LOW_BINS)
    {
        for (size_t i = 0; i < NUM_LOW_BINS; i++)
        {
            lowFrequencyWeights[i] = lowWeights[i];
        }
    }

    if (midWeights && midWeights.size() == NUM_MID_BINS)
    {
        for (size_t i = 0; i < NUM_MID_BINS; i++)
        {
            midFrequencyWeights[i] = midWeights[i];
        }
    }

    if (highWeights && highWeights.size() == NUM_HIGH_BINS)
    {
        for (size_t i = 0; i < NUM_HIGH_BINS; i++)
        {
            highFrequencyWeights[i] = highWeights[i];
        }
    }

    AudioConfiguration_S audioConfig;
    audioConfig.mode = mode;
    audioConfig.allowMultipleActive = allowMultipleActive;
    
    memcpy(audioConfig.frequencyFlags, frequencyFlags, sizeof(frequencyFlags));
    memcpy(audioConfig.lowFrequencyWeights, lowFrequencyWeights, sizeof(lowFrequencyWeights));
    memcpy(audioConfig.midFrequencyWeights, midFrequencyWeights, sizeof(midFrequencyWeights));
    memcpy(audioConfig.highFrequencyWeights, highFrequencyWeights, sizeof(highFrequencyWeights));
    memcpy(audioConfig.magnitudeThresholds, magnitudeThresholds, sizeof(magnitudeThresholds));
    
    if (m_pAudioOrchestrator)
    {
       setAudioConfig(audioConfig);
    }

    float fastAlpha = doc["audioFastAlpha"];
    float slowAlpha = doc["audioSlowAlpha"];
    // Smoothing factors get sent directly to the FFT analyzer
    if (m_pFftAnalyzer)
    {
        m_pFftAnalyzer->setFastAlpha(fastAlpha);
        m_pFftAnalyzer->setSlowAlpha(slowAlpha);
    }
    // TODO: re-enable/re-test EEPROM
    // saveAudioConfigToEEPROM(audioConfig);
    return true;
}

void HW_NeonManager::publishManagerStatus(void)
{
    memset(m_managerStatusMessage, 0, sizeof(m_managerStatusMessage));
    m_managerStatusBuffer.clear();
    m_managerStatusBuffer["type"] = "manager_status";
    m_managerStatusBuffer["status"] = "online";
    
    // Add client lock info
    m_managerStatusBuffer["locked"] = m_clientLocked;
    if (m_clientLocked)
    {
        m_managerStatusBuffer["lockedBy"] = m_authorizedClientId;
        m_managerStatusBuffer["lockTimeRemaining"] = (LOCK_TIMEOUT_MS - (millis() - m_lockTimestamp)) / 1000;
    }

    JsonArray controllers = m_managerStatusBuffer.createNestedArray("controllers");
    for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    {
        JsonObject controller = controllers.createNestedObject();
        controller["connected"] = true;\
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                DeviceAddresses::MANAGER_MAC[0], DeviceAddresses::MANAGER_MAC[1], 
                DeviceAddresses::MANAGER_MAC[2], DeviceAddresses::MANAGER_MAC[3], 
                DeviceAddresses::MANAGER_MAC[4], DeviceAddresses::MANAGER_MAC[5]);
        controller["mac"] = macStr;

        for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
        {
            JsonObject supply = controller.createNestedObject("supply");
            supply["id"] = m_suppliesState[supplyIdx].id;
            supply["enable"] = m_suppliesState[supplyIdx].enable;
            supply["brightness"] = m_suppliesState[supplyIdx].brightness;
            supply["current"] = m_suppliesState[supplyIdx].current;
        }
        controller["distance"] = m_distance;
        controller["profileActive"] = m_profileActive;
    }
    serializeJson(m_managerStatusBuffer, m_managerStatusMessage);

    if (!m_mqttClient.publish(MQTT_MANAGER_TOPIC_PREFIX, m_managerStatusMessage))
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Failed to publish manager status: %d", m_mqttClient.state());
        m_pUtils->logError(buffer);
    }
}


void HW_NeonManager::reconnectMQTT(void)
{
    while (!m_mqttClient.connected())
    {
        // Set last will message using the MQTT library's standard connect parameters
        // PubSubClient does not have a separate setWill method - will message is set in connect
        const char* willTopic = MQTT_MANAGER_TOPIC_PREFIX;
        const char* willMessage = "{\"status\":\"offline\"}";
        if (m_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, willTopic, 1, true, willMessage))
        {
            if (!m_mqttClient.subscribe(MQTT_PROFILE_TOPIC))
            {
                m_pUtils->logWarning("Failed to subscribe to profile topic");
            }
            if (!m_mqttClient.subscribe(MQTT_AUDIO_CONFIG_TOPIC))
            {
                m_pUtils->logWarning("Failed to subscribe to audio_config topic");
            }
            
            // Subscribe to lock request topic
            if (!m_mqttClient.subscribe(MQTT_LOCK_REQUEST_TOPIC))
            {
                m_pUtils->logWarning("Failed to subscribe to lock request topic");
            }
            
            char manualTopic[50];
            for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
            {
                snprintf(manualTopic, sizeof(manualTopic), "%s%d", MQTT_MANUAL_PREFIX, idx);
                if(!m_mqttClient.subscribe(manualTopic))
                {
                    Serial.println("Failed to subscribe to manual topic");
                }
            }
            
            publishManagerStatus();
        }
        else
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "failed, rc=%d", m_mqttClient.state());
            m_pUtils->logWarning(buffer);
            m_pUtils->logWarning(" try again in 5 seconds");
            delay(5000);
        }
    }
}
void HW_NeonManager::handleMqttMessage(char* topic, byte* payload, unsigned int length)
{
    char* message = new char[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    if (strcmp(topic, MQTT_PROFILE_TOPIC) == 0)
    {
        m_profileConfigBuffer.clear();
        deserializeJson(m_profileConfigBuffer, message);
        
        const char* clientId = m_profileConfigBuffer["clientId"] | "";
        
        if (!isClientAuthorized(clientId))
        {
            m_pUtils->logWarning("Profile command rejected: Unauthorized client");
            return;
        }

        if (!parseProfileConfig(m_profileConfigBuffer))
        {
            m_pUtils->logWarning("Failed to parse profile config");
        }
    }
    else if (strcmp(topic, MQTT_AUDIO_CONFIG_TOPIC) == 0)
    {
        m_audioConfigBuffer.clear();
        deserializeJson(m_audioConfigBuffer, message);
        
        const char* clientId = m_audioConfigBuffer["clientId"] | "";
        
        if (!isClientAuthorized(clientId))
        {
            m_pUtils->logWarning("Audio config command rejected: Unauthorized client");
            return;
        }
        
        if (!parseAudioConfig(m_audioConfigBuffer))
        {
            m_pUtils->logWarning("Failed to parse audio config");
        }
    }
    else if (strstr(topic, MQTT_MANUAL_PREFIX) != NULL)
    {
        // Extract controller index from topic - topic is project/manual/<controller_id>
        char* controllerIdStr = topic + strlen(MQTT_MANUAL_PREFIX);
        size_t controllerId = atoi(controllerIdStr);

        m_manualConfigBuffer.clear();
        deserializeJson(m_manualConfigBuffer, message);
        
        const char* clientId = m_manualConfigBuffer["clientId"] | "";
        
        if (!isClientAuthorized(clientId))
        {
            m_pUtils->logWarning("manual command rejected: Unauthorized client");
            return;
        }
        
        if (!parseManualConfig(m_manualConfigBuffer, controllerId))
        {
            m_pUtils->logWarning("Failed to parse manual config");
        }
    }
    else if (strcmp(topic, MQTT_LOCK_REQUEST_TOPIC) == 0)
    {
        m_lockConfigBuffer.clear();
        deserializeJson(m_lockConfigBuffer, message);
        
        const char* action = m_lockConfigBuffer["action"] | "";
        const char* clientId = m_lockConfigBuffer["clientId"] | "";
        
        if (strcmp(action, "lock") == 0)
        {
            bool success = requestClientLock(clientId);
            publishLockResponse(clientId, success);
        } 
        else if (strcmp(action, "unlock") == 0)
        {
            if (m_clientLocked && m_authorizedClientId == clientId)
            {
                bool success = releaseClientLock(clientId);
                publishLockResponse(clientId, success);
            }
            else
            {
                publishLockResponse(clientId, false);
            }
        }
        else if (strcmp(action, "heartbeat") == 0)
        {
            if (m_clientLocked && m_authorizedClientId == clientId)
            {
                m_lockTimestamp = millis();
                publishLockResponse(clientId, true);
            }
        }
    }
    
    delete[] message;
}

void HW_NeonManager::initializeI2C(void)
{
    bool i2cDistanceSensing = (PIN_DISTANCE_AIN == -1);
    Wire.begin(PIN_DAC_SDA, PIN_DAC_SCL);
    Wire.setClock(I2C_SPEED);

    if (i2cDistanceSensing)
    {
        Wire2.begin(PIN_DISTANCE_SENSE_SDA, PIN_DISTANCE_SENSE_SCL);
        Wire2.setClock(I2C_SPEED);
    }
    
    // Uses Wire (bus 0) by default
    if (!m_dac.begin())
    {
        if (Serial) Serial.println("Failed to initialize DAC");
    }

    // Uses Wire2 (bus 1)
    if (!m_distanceSensor.begin(&Wire2, i2cDistanceSensing))
    {
        if (Serial) Serial.println("Failed to initialize distance sensor");
    }
}

void HW_NeonManager::initializeSupplies(void)
{
    pinMode(PIN_SUPPLY_0_EN, OUTPUT);
    pinMode(PIN_SUPPLY_1_EN, OUTPUT);

    //hardcode for now until we have a generalized way to set the supply output type
    m_supplyOutputType[0] = SupplyOutputType_E::PWM;
    m_supplyOutputType[1] = SupplyOutputType_E::DAC;

    for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
    {
        setSupplyEnable(idx, false);
        applySupplyEnable(idx);
    }
}


void HW_NeonManager::applySupplyEnable(size_t idx)
{
    bool enable = getSupplyEnable(idx);
    if (idx == 0)
    {
        digitalWrite(PIN_SUPPLY_0_EN, enable);
    }
    else if (idx == 1)
    {
        digitalWrite(PIN_SUPPLY_1_EN, enable);
    }
}

void HW_NeonManager::applySupplyBrightness(size_t idx)
{
    SupplyOutputType_E outputType = getSupplyOutputType(idx);   
    float brightness = getSupplyBrightness(idx);
    if (outputType == SupplyOutputType_E::PWM)
    {
        updateRmtPwmDutyCycle(brightness);
    }
    else if (outputType == SupplyOutputType_E::DAC)
    {
        uint16_t dacValue = static_cast<uint16_t>(brightness * HW::BIT_RESOLUTION_12);
        m_dac.setChannelValue(MCP4728_CHANNEL_A, dacValue);
    }
}

void HW_NeonManager::applyNeonSettings(void)
{
    for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
    {
        applySupplyEnable(idx);
        applySupplyBrightness(idx);
    }
}

void HW_NeonManager::updateDistanceValue(void)
{
    uint16_t distance = m_distanceSensor.readDistanceCm();
    setDistance(static_cast<float>(distance));
}

void HW_NeonManager::initializeRmtPwm(void)
{
    rmt_config_t config = {};
    config.rmt_mode = RMT_MODE_TX;
    config.channel = RMT_CHANNEL_0;
    config.gpio_num = static_cast<gpio_num_t>(PIN_SUPPLY_0_PWM_OUT);
    config.mem_block_num = 1;
    config.clk_div = HW::RMT_CLK_DIV;  // 80MHz / 80 = 1MHz -> 1us resolution
    config.tx_config.loop_en = true;
    config.tx_config.carrier_en = false; 
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    
    esp_err_t err = rmt_config(&config);
    if (err != ESP_OK)
    {
        m_pUtils->logWarning("Failed to configure RMT");
    }
    err = rmt_driver_install(config.channel, 0, 0);
    if (err != ESP_OK)
    {
        m_pUtils->logWarning("Failed to install RMT driver");
    }
    
    updateRmtPwmDutyCycle(0.0);
}

void HW_NeonManager::updateRmtPwmDutyCycle(float brightness)
{
    // Times in us
    uint32_t onTime = brightness * HW::TOTAL_PERIOD_US;
    uint32_t offTime = HW::TOTAL_PERIOD_US - onTime;
    
    rmt_item32_t items[1];
    
    // Configure high and low durations
    items[0].duration0 = onTime;
    items[0].level0 = 0;
    items[0].duration1 = offTime;
    items[0].level1 = 1;
    
    if (brightness <= 0.01)
    {
        items[0].duration0 = 1;
        items[0].level0 = 1;
        items[0].duration1 = HW::TOTAL_PERIOD_US - 1;
        items[0].level1 = 1;
    }
    else if (brightness >= 0.99)
    {
        items[0].duration0 = HW::TOTAL_PERIOD_US - 1;
        items[0].level0 = 0;
        items[0].duration1 = 1;
        items[0].level1 = 0;
    }
    
    rmt_write_items(RMT_CHANNEL_0, items, 1, true);
}

bool HW_NeonManager::requestClientLock(const char* clientId)
{
    // If no client is locked or this client has the lock, grant the lock
    if (!m_clientLocked || m_authorizedClientId.equals(clientId))
    {
        m_clientLocked = true;
        m_authorizedClientId = String(clientId);
        m_lockTimestamp = millis();
        
        return true;
    }
    
    return false;
}

bool HW_NeonManager::releaseClientLock(const char* clientId)
{
    // only the authorized client can release the lock
    if (m_clientLocked && m_authorizedClientId.equals(clientId))
    {
        m_clientLocked = false;
        m_authorizedClientId = "";
        
        return true;
    }
    
    return false;
}

bool HW_NeonManager::isClientAuthorized(const char* clientId)
{
    if (clientId == nullptr || strlen(clientId) == 0)
    {
        return false;
    }
    
    if (m_clientLocked)
    {
        return (m_authorizedClientId == clientId);
    }
    else
    {
        return false;
    }
}

void HW_NeonManager::checkLockTimeout()
{
    if (m_clientLocked && ((millis() - m_lockTimestamp) > LOCK_TIMEOUT_MS))
    {        
        m_clientLocked = false;
        m_authorizedClientId = "";
    }
}

void HW_NeonManager::publishLockResponse(const char* clientId, bool success)
{
    m_lockConfigBuffer.clear();
    m_lockConfigBuffer["clientId"] = clientId;
    m_lockConfigBuffer["status"] = success ? "granted" : "denied";
    
    if (success)
    {
        m_lockConfigBuffer["timeoutMs"] = LOCK_TIMEOUT_MS;
    }
    
    char response[256];
    serializeJson(m_lockConfigBuffer, response);
    
    m_mqttClient.publish(MQTT_LOCK_RESPONSE_TOPIC, response);
}

// TODO: re-enable/re-test EEPROM
void HW_NeonManager::saveAudioConfigToEEPROM(AudioConfiguration_S audioConfig)
{
    // m_nonVolatileStorage.putUChar("audio_mode", static_cast<uint8_t>(mode));
    // m_nonVolatileStorage.putBool("multi_active", allowMultipleActive);
    // m_nonVolatileStorage.putFloat("fast_alpha", fastAlpha);
    // m_nonVolatileStorage.putFloat("slow_alpha", slowAlpha);
    
    // // Save controller configs
    // for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    // {
    //     char keyFreq[16];
    //     char keyThresh[16];
        
    //     snprintf(keyFreq, sizeof(keyFreq), "freq_flag_%d", idx);
    //     snprintf(keyThresh, sizeof(keyThresh), "mag_thresh_%d", idx);
        
    //     m_nonVolatileStorage.putUChar(keyFreq, static_cast<uint8_t>(controllerConfigs[idx].frequencyFlags));
    //     m_nonVolatileStorage.putFloat(keyThresh, controllerConfigs[idx].magnitudeThreshold);
    // }
    
    // m_pUtils->logWarning("Audio configuration saved to EEPROM");
}

void HW_NeonManager::loadNonVolatileSettings(void)
{
    // // Check if we have saved audio settings
    // if (m_nonVolatileStorage.isKey("audio_mode"))
    // {
    //     m_pUtils->logWarning("Loading audio configuration from EEPROM");
        
    //     AudioAssignmentMode_E mode = static_cast<AudioAssignmentMode_E>(
    //         m_nonVolatileStorage.getUChar("audio_mode", static_cast<uint8_t>(AudioAssignmentMode_E::FIXED)));
        
    //     bool allowMultipleActive = m_nonVolatileStorage.getBool("multi_active", false);
    //     float fastAlpha = m_nonVolatileStorage.getFloat("fast_alpha", 0.3f);
    //     float slowAlpha = m_nonVolatileStorage.getFloat("slow_alpha", 0.01f);
        
    //     // Load controller configs
    //     ControllerAudioConfig_S controllerConfigs[NUM_CONTROLLERS];
    //     for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //     {
    //         char keyFreq[16];
    //         char keyThresh[16];
            
    //         snprintf(keyFreq, sizeof(keyFreq), "freq_flag_%d", idx);
    //         snprintf(keyThresh, sizeof(keyThresh), "mag_thresh_%d", idx);
            
    //         controllerConfigs[idx].frequencyFlags = static_cast<FrequencyBand_E>(
    //             m_nonVolatileStorage.getUChar(keyFreq, static_cast<uint8_t>(FrequencyBand_E::MID_FREQ)));
            
    //         controllerConfigs[idx].magnitudeThreshold = m_nonVolatileStorage.getFloat(keyThresh, 0.25f);
    //     }
        
    //     if (m_pFftAnalyzer)
    //     {
    //         m_pFftAnalyzer->setFastAlpha(fastAlpha);
    //         m_pFftAnalyzer->setSlowAlpha(slowAlpha);
    //     }
        
    //     if (m_pAudioOrchestrator)
    //     {
    //         setAudioConfig(mode, allowMultipleActive, controllerConfigs);
    //     }
    // }
    // else
    // {
    //     m_pUtils->logWarning("No saved audio configuration found");
    // }
}
