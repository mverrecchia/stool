// hardware/shared/include/DeviceAddresses.h
#pragma once
#include <stdint.h>

#define MAC_ADDRESS_SIZE 6

namespace DeviceAddresses {
    constexpr uint8_t MANAGER_MAC[] = {0x24, 0x0A, 0xC4, 0x0D, 0x81, 0x40};
}