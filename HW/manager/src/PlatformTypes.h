#pragma once

#include "PlatformConstants.h"
#include "PlatformTimer.h"
#include <cstdint>
#include <cmath>

enum class ManagerState_E
{
    IDLE           = 0,
    PAIRING        = 1,
    ACTIVE_PARTIAL = 2,
    ACTIVE_FULL    = 3,
};

enum class ControllerState_E
{
    IDLE     = 0,
    UNPAIRED = 1,
    PAIRED   = 2,
};

enum class NeonChannel_E
{
    ROTATING   = 0,
    STATIONARY = 1,
};

enum class ProfileType_E
{
    COS         = 0,
    BOUNCE      = 1,
    EXPONENTIAL = 2,
    PULSE       = 3,
    TRIANGLE    = 4,
    ELASTIC     = 5,
    CASCADE     = 6,
    FLICKER     = 7,
};

enum class AudioAssignmentMode_E
{
    FIXED      = 0,
    RANDOM     = 1,
    SEQUENTIAL = 2,
};

enum class NetworkModeType_E
{
    AUTO   = 0,
    MANUAL = 1,
};

enum class BroadcastType_E
{
    BROADCAST = 0,
    UNICAST   = 1,
};

enum class SupplyOutputType_E
{
    PWM = 0,
    DAC = 1,
};

enum FrequencyBand_E
{
    NONE_FREQ = 0x00,
    LOW_FREQ  = 0x01,
    MID_FREQ  = 0x02,
    HIGH_FREQ = 0x04
};

enum class DistanceOverrideState_E
{
    INACTIVE     = 0,
    RISING_EDGE  = 1,
    ACTIVE       = 2,
    FALLING_EDGE = 3
};

struct AudioConfiguration_S
{
    AudioAssignmentMode_E mode;
    bool allowMultipleActive;
    FrequencyBand_E frequencyFlags[NUM_CONTROLLERS][NUM_SUPPLIES_PER_CONTROLLER];
    float lowFrequencyWeights[NUM_LOW_BINS];
    float midFrequencyWeights[NUM_MID_BINS];
    float highFrequencyWeights[NUM_HIGH_BINS];
    float magnitudeThresholds[NUM_AUDIO_BUCKETS];
};

struct __attribute__((packed)) SupplyParameters_S
{
    uint8_t id;
    bool enable;
    float brightness;
    float current;
};

struct __attribute__((packed)) MotorParameters_S
{
    float speed;
    float acceleration;
    float maxSpeed;
    bool enable;
    bool direction;
};

struct __attribute__((packed)) ProfileRequestParameters_S
{
    ProfileType_E type;  // Profile type
    float magnitude;     // Effect magnitude (0-1)
    float frequency;     // For oscillating patterns (Hz)
    float phase;         // Phase offset for oscillating patterns
    bool enable;         // Enable the profile
    bool stopProfile;    // Stop the profile
};

struct __attribute__((packed)) ControllerRequestParameters_S
{
    SupplyParameters_S supplies[NUM_SUPPLIES_PER_CONTROLLER];
    MotorParameters_S motor;
};

struct __attribute__((packed)) ControllerStateParameters_S
{
    SupplyParameters_S supplies[NUM_SUPPLIES_PER_CONTROLLER];
    MotorParameters_S motor;
    bool profileActive;
    float distance;
};