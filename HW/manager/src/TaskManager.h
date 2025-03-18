// TaskManager.h
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "HW_NeonManager.h"
#include <Arduino.h>

namespace TaskManager {
    static constexpr uint32_t MANAGER_UPDATE_FREQ_HZ = 100;
    static constexpr uint32_t MANAGER_PERIOD_MS = 1000 / MANAGER_UPDATE_FREQ_HZ;
    
    static constexpr uint32_t MANAGER_STACK_SIZE = 8192;
    
    static constexpr uint32_t MANAGER_PRIORITY = 10;
    static TaskHandle_t managerUpdateTaskHandle = nullptr;

    void managerUpdateTask(void* parameter) 
    {
        HW_NeonManager* manager = static_cast<HW_NeonManager*>(parameter);
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(MANAGER_PERIOD_MS);
        float deltaTimeSeconds = MANAGER_PERIOD_MS / 1000.0f;

        for(;;)
        {
            manager->update(deltaTimeSeconds);
            vTaskDelayUntil(&xLastWakeTime, period);
        }
    }

    void createManagerUpdateTask(HW_NeonManager* pManager)
    {
        xTaskCreate(
            TaskManager::managerUpdateTask,
            "ManagerTask",
            TaskManager::MANAGER_STACK_SIZE,
            pManager,
            TaskManager::MANAGER_PRIORITY,
            &managerUpdateTaskHandle
        );
    }
}