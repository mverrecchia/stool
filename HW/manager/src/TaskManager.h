// TaskManager.h
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "HW_NeonManager.h"
#include "HW_FFTAudioAnalyzer.h"
#include <Arduino.h>

namespace TaskManager {
    // Core assignments
    static constexpr BaseType_t NETWORK_CORE = 0;
    static constexpr BaseType_t PROCESSING_CORE = 1;
    
    // Task priorities
    static constexpr uint32_t MQTT_TASK_PRIORITY = 2;
    static constexpr uint32_t MANAGER_UPDATE_PRIORITY = 2;

    // Stack sizes
    static constexpr uint32_t MQTT_STACK_SIZE = 4096;
    static constexpr uint32_t MANAGER_STACK_SIZE = 8192;

    // Task frequencies
    static constexpr uint32_t MQTT_UPDATE_FREQ_HZ = 10;
    static constexpr uint32_t MANAGER_UPDATE_FREQ_HZ = 100;

    // Task periods
    static constexpr uint32_t MQTT_PERIOD_MS = 1000 / MQTT_UPDATE_FREQ_HZ;
    static constexpr uint32_t MANAGER_PERIOD_MS = 1000 / MANAGER_UPDATE_FREQ_HZ;

    // Event bits for event group
    static constexpr EventBits_t SENSOR_UPDATED_BIT = (1 << 1);
    static constexpr EventBits_t SUPPLY_UPDATED_BIT = (1 << 2);
    
    // Task handles
    extern TaskHandle_t mqttTaskHandle;
    extern TaskHandle_t managerUpdateTaskHandle;

    // Synchronization primitives
    extern SemaphoreHandle_t mqttMutex;
    extern SemaphoreHandle_t mqttPublishMutex;
    extern SemaphoreHandle_t stateMutex;

    // Task implementations
    void mqttTask(void* parameter);
    void managerUpdateTask(void* parameter);

    // System initialization
    void initializeTaskSystem();
    
    // Task creation functions
    void createMqttTask(HW_NeonManager* pManager);
    void createManagerUpdateTask(HW_NeonManager* pManager);
    
    // Initialize all tasks
    void initializeTasks(HW_NeonManager* pManager);
}