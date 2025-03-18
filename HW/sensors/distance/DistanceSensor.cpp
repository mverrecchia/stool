// DistanceSensor.cpp
#include "DistanceSensor.h"

DistanceSensor::DistanceSensor()
    : m_wire(nullptr)
    , m_shift(0)
    , m_initialized(false)
{
}

bool DistanceSensor::begin(TwoWire *wire)
{
    m_wire = wire;
    // Make sure we have a valid Wire instance
    if (!m_wire) {
        return false;
    }
    
    // Try to read the shift value
    m_wire->beginTransmission(SENSOR_ADDRESS);
    m_wire->write(SHIFT_ADDR);
    if (m_wire->endTransmission() != 0) {
        return false;  // Transmission error
    }
    
    m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
    if (1 <= m_wire->available()) {
        m_shift = m_wire->read();
        m_initialized = true;
        return true;
    }
    
    return false;  // Failed to read shift value
}

uint8_t DistanceSensor::readDistanceCm()
{
    if (!m_initialized) {
        return 0;  // Return 0 if not initialized
    }
    
    uint8_t distanceRaw[2] = {0};
    if (readRawDistance(distanceRaw)) {
        // Calculate distance in cm using the formula from the example
        return (distanceRaw[0] * 16 + distanceRaw[1]) / 16 / (int)pow(2, m_shift);
    }
    
    return 0;  // Return 0 if read failed
}

bool DistanceSensor::readRawDistance(uint8_t *distanceRaw)
{
    if (!m_initialized || !distanceRaw) {
        return false;
    }
    
    m_wire->beginTransmission(SENSOR_ADDRESS);
    m_wire->write(DISTANCE_ADDR);
    if (m_wire->endTransmission() != 0) {
        return false;  // Transmission error
    }
    
    m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)2);
    if (2 <= m_wire->available()) {
        distanceRaw[0] = m_wire->read();
        distanceRaw[1] = m_wire->read();
        return true;
    }
    
    return false;  // Failed to read distance data
}

bool DistanceSensor::readLeftEdge(uint8_t &value)
{
    if (!m_initialized) {
        return false;
    }
    
    m_wire->beginTransmission(SENSOR_ADDRESS);
    m_wire->write(LEFT_EDGE_ADDR);
    if (m_wire->endTransmission() != 0) {
        return false;
    }
    
    m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
    if (1 <= m_wire->available()) {
        value = m_wire->read();
        return true;
    }
    
    return false;
}

bool DistanceSensor::readRightEdge(uint8_t &value)
{
    if (!m_initialized) {
        return false;
    }
    
    m_wire->beginTransmission(SENSOR_ADDRESS);
    m_wire->write(RIGHT_EDGE_ADDR);
    if (m_wire->endTransmission() != 0) {
        return false;
    }
    
    m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
    if (1 <= m_wire->available()) {
        value = m_wire->read();
        return true;
    }
    
    return false;
}

bool DistanceSensor::readPeakEdge(uint8_t &value)
{
    if (!m_initialized) {
        return false;
    }
    
    m_wire->beginTransmission(SENSOR_ADDRESS);
    m_wire->write(PEAK_EDGE_ADDR);
    if (m_wire->endTransmission() != 0) {
        return false;
    }
    
    m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
    if (1 <= m_wire->available()) {
        value = m_wire->read();
        return true;
    }
    
    return false;
}