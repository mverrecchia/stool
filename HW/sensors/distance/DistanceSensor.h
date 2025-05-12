// DistanceSensor.h
#pragma once

#include <Arduino.h>
#include <Wire.h>

class DistanceSensor {
public:
    DistanceSensor()
        : m_wire(nullptr)
        , m_shift(0)
        , m_initialized(false)
    {
    }
    
    bool begin(TwoWire *wire, bool i2cEnabled);
    uint16_t readDistanceCm();
    bool readRawDistance(uint8_t *distanceRaw);
    bool readLeftEdge(uint8_t &value);
    bool readRightEdge(uint8_t &value);
    bool readPeakEdge(uint8_t &value);

private:
    static constexpr uint8_t SENSOR_ADDRESS = (0x80 >> 1);
    static constexpr uint8_t SHIFT_ADDR = 0x35;
    static constexpr uint8_t DISTANCE_ADDR = 0x5E;
    static constexpr uint8_t RIGHT_EDGE_ADDR = 0xF8;
    static constexpr uint8_t LEFT_EDGE_ADDR = 0xF9;
    static constexpr uint8_t PEAK_EDGE_ADDR = 0xFA;
    
    TwoWire *m_wire;
    uint8_t m_shift;
    bool m_initialized;
    bool m_i2cEnabled;
};