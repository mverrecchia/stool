// TaskManager.cpp
#include "TaskManager.h"

namespace TaskManager {
    // Initialize static member variables
    TaskHandle_t mqttTaskHandle = nullptr;
    TaskHandle_t managerUpdateTaskHandle = nullptr;

    // Synchronization primitives
    SemaphoreHandle_t mqttMutex = nullptr;
    SemaphoreHandle_t mqttPublishMutex = nullptr;
    SemaphoreHandle_t stateMutex = nullptr;

    void initializeTaskSystem()
    {
        mqttMutex = xSemaphoreCreateMutex();
        mqttPublishMutex = xSemaphoreCreateMutex();
        stateMutex = xSemaphoreCreateMutex();
    }

    void createMqttTask(HW_NeonManager* pManager)
    {
        xTaskCreatePinnedToCore(
            mqttTask,
            "MQTTTask",
            MQTT_STACK_SIZE,
            pManager,
            MQTT_TASK_PRIORITY,
            &mqttTaskHandle,
            NETWORK_CORE
        );
    }

    void createManagerUpdateTask(HW_NeonManager* pManager)
    {
        xTaskCreatePinnedToCore(
            managerUpdateTask,
            "ManagerTask",
            MANAGER_STACK_SIZE,
            pManager,
            MANAGER_UPDATE_PRIORITY,
            &managerUpdateTaskHandle,
            PROCESSING_CORE
        );
    }

    void initializeTasks(HW_NeonManager* pManager)
    {        
        createMqttTask(pManager);
        createManagerUpdateTask(pManager);
    }

    void mqttTask(void* parameter)
    {
        HW_NeonManager* manager = static_cast<HW_NeonManager*>(parameter);
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(MQTT_PERIOD_MS);

        TickType_t lastStatusTime = 0;
        const TickType_t STATUS_PUBLISH_PERIOD = pdMS_TO_TICKS(1000); // 1 second

        for(;;)
        {
            if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                manager->handleMqtt();
                xSemaphoreGive(mqttMutex);
            }

            TickType_t currentTime = xTaskGetTickCount();
            if ((currentTime - lastStatusTime) >= STATUS_PUBLISH_PERIOD)
            {
                lastStatusTime = currentTime;
                
                if (xSemaphoreTake(mqttPublishMutex, pdMS_TO_TICKS(10)) == pdTRUE)
                {
                    manager->publishManagerStatus();
                    xSemaphoreGive(mqttPublishMutex);
                }
            }
            
            vTaskDelayUntil(&xLastWakeTime, period);
        }
    }

    void managerUpdateTask(void* parameter)
    {
        HW_NeonManager* manager = static_cast<HW_NeonManager*>(parameter);
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(MANAGER_PERIOD_MS);
        float deltaTimeSeconds = MANAGER_PERIOD_MS / 1000.0f;

        for(;;)
        {
            if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                manager->update(deltaTimeSeconds);
                xSemaphoreGive(stateMutex);
            }
            
            vTaskDelayUntil(&xLastWakeTime, period);
        }
    }
}