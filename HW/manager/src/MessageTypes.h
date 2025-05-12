#pragma once

#include "PlatformTypes.h"
#include "PlatformConstants.h"

enum class MessageType_E : uint8_t
{
    HEARTBEAT_M = 0x00,
    MANUAL = 0x01,
    PROFILE = 0x02,
    AUDIO = 0x03,
    HEARTBEAT_C = 0x10,
};

// Heartbeat message sent at 10Hz to all controllers
struct __attribute__((packed)) ManagerHeartbeatMessage_S
{
    union
    {
        struct
        {
            uint8_t expectedControllerCount : 3;  // Expected number of controllers
            uint8_t totalControllerCount    : 3;  // Total number of controllers
            uint8_t reserved                : 2;
        };
        uint8_t networkStatus;
    };
};

struct __attribute__((packed)) ManualMessage_S
{
    SupplyParameters_S supplies[NUM_SUPPLIES_PER_CONTROLLER];
    MotorParameters_S motor;
};

struct __attribute__((packed)) AudioMessage_S 
{
    uint8_t controllerMac[MAC_ADDRESS_SIZE];
    uint8_t frequencyFlags[NUM_SUPPLIES_PER_CONTROLLER];
    float prevailingWeightedLowMagnitude;
    float prevailingWeightedMidMagnitude;
    float prevailingWeightedHighMagnitude;
    bool motorEnable;
};

struct __attribute__((packed)) ControllerHeartbeatMessage_S
{
    SupplyParameters_S supplies[NUM_SUPPLIES_PER_CONTROLLER];
    MotorParameters_S motor;
    bool profileActive;
    float supplyCurrent[NUM_SUPPLIES_PER_CONTROLLER];
    float distance;
};

struct __attribute__((packed)) ManagerHeartbeatMessageFull_S
{
    MessageType_E header = MessageType_E::HEARTBEAT_M;
    ManagerHeartbeatMessage_S heartbeat;
};

struct __attribute__((packed)) ManualMessageFull_S
{
    MessageType_E header = MessageType_E::MANUAL;
    ManualMessage_S manual;
};

struct __attribute__((packed)) ProfileMessageFull_S
{
    MessageType_E header = MessageType_E::PROFILE;
    ProfileRequestParameters_S profileParams;
};

struct __attribute__((packed)) AudioMessageFull_S
{
    MessageType_E header = MessageType_E::AUDIO;
    AudioMessage_S audio[NUM_CONTROLLERS];
};

struct __attribute__((packed)) ControllerHeartbeatMessageFull_S
{
    MessageType_E header = MessageType_E::HEARTBEAT_C;
    ControllerHeartbeatMessage_S heartbeat;
};

