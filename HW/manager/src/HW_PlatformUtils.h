// hardware/shared/include/HW_PlatformUtils.h
#pragma once
#include "PlatformUtils.h"
#include <Arduino.h>

class HW_PlatformUtils : public PlatformUtils {
public:
    float mapRange(float value, float inMin, float inMax, float outMin, float outMax) override
    {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }

    float clamp(float value, float min, float max) override
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    void log(const char* message) override
    {
        if(Serial)
        {
            Serial.println(message);
        }
    }

    void logFloat(float value) override
    {
        if(Serial)
        {
            Serial.println(value);
        }
    }

    void logWarning(const char* message) override
    {
        if (Serial)
        {
            Serial.print("WARNING: ");
            Serial.println(message);
        }
    }

    void logError(const char* message) override
    {
        if (Serial)
        {
            Serial.print("ERROR: ");
            Serial.println(message);
        }
    }
};