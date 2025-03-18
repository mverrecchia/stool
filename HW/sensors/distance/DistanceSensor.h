// DistanceSensor.h
#pragma once

#include <Arduino.h>
#include <Wire.h>

class DistanceSensor {
public:
    // Constructor - allows specifying which Wire instance to use
    DistanceSensor();
    
    // Initialize the sensor
    bool begin(TwoWire *wire);
    
    // Read distance measurement in centimeters
    uint8_t readDistanceCm();
    
    // Read raw distance values
    bool readRawDistance(uint8_t *distanceRaw);
    
    // Read edge data (optional functionality)
    bool readLeftEdge(uint8_t &value);
    bool readRightEdge(uint8_t &value);
    bool readPeakEdge(uint8_t &value);

private:
    // Constants for sensor operation
    static constexpr uint8_t SENSOR_ADDRESS = (0x80 >> 1);
    static constexpr uint8_t SHIFT_ADDR = 0x35;
    static constexpr uint8_t DISTANCE_ADDR = 0x5E;
    static constexpr uint8_t RIGHT_EDGE_ADDR = 0xF8;
    static constexpr uint8_t LEFT_EDGE_ADDR = 0xF9;
    static constexpr uint8_t PEAK_EDGE_ADDR = 0xFA;
    
    // Reference to the Wire instance to use
    TwoWire *m_wire;
    
    // Shift value needed for calculations
    uint8_t m_shift;
    
    // Status flag
    bool m_initialized;
};