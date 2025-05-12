// shared/include/PlatformUtils.h
#include "PlatformTypes.h"

#pragma once

class PlatformUtils {
public:
    virtual ~PlatformUtils() = default;

    // Math operations
    float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    virtual float mapRange(float value, float inMin, float inMax, float outMin, float outMax) = 0;
    virtual float clamp(float value, float min, float max) = 0;

    // Logging operations
    virtual void log(const char* message) = 0;
    virtual void logFloat(float value) = 0;
    virtual void logWarning(const char* message) = 0;
    virtual void logError(const char* message) = 0;
};