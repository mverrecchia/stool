#include "DistanceSensor.h"

// Two types of distance sensors - I2C and analog
// Not separated very well but works for now
bool DistanceSensor::begin(TwoWire *wire, bool i2cEnabled)
{
    if (i2cEnabled)
    {
        m_i2cEnabled = true;
        m_wire = wire;

        m_wire->beginTransmission(SENSOR_ADDRESS);
        m_wire->write(SHIFT_ADDR);
        if (m_wire->endTransmission() != 0)
        {
            return false;
        }
        
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
        if (1 <= m_wire->available())
        {
            m_shift = m_wire->read();
            m_initialized = true;
            return true;
        }
    }
    else
    {
        pinMode(PIN_DISTANCE_AIN, INPUT);
        m_i2cEnabled = false;
        m_initialized = true;
        return true;
    }
    
    return false;
}

uint16_t DistanceSensor::readDistanceCm()
{
    uint16_t distance = 0;
    if (!m_initialized)
    {
        return 0;
    }
    if (m_i2cEnabled)
    {
        uint8_t distanceRaw[2] = {0};
        if (readRawDistance(distanceRaw))
        {
            distance = (distanceRaw[0] * 16 + distanceRaw[1]) / 16 / (int)pow(2, m_shift);
        }
    }
    else
    {
        distance = analogRead(PIN_DISTANCE_AIN);
    }
    
    return distance;
}

bool DistanceSensor::readRawDistance(uint8_t *distanceRaw)
{
    bool success = false;
    
    if (m_initialized && m_wire)
    {
        m_wire->beginTransmission(SENSOR_ADDRESS);
        m_wire->write(DISTANCE_ADDR);
        if (m_wire->endTransmission() != 0)
        {
            return success;
        }
        
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)2);
        if (2 <= m_wire->available())
        {
            distanceRaw[0] = m_wire->read();
            distanceRaw[1] = m_wire->read();
            success = true;
        }
    }
    
    return success;
}

bool DistanceSensor::readLeftEdge(uint8_t &value)
{
    if (m_initialized)
    {
        m_wire->beginTransmission(SENSOR_ADDRESS);
        m_wire->write(LEFT_EDGE_ADDR);
        if (m_wire->endTransmission() != 0)
        {
            return false;
        }
        
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
        if (1 <= m_wire->available())
        {
            value = m_wire->read();
            return true;
        }
    }
    
    return false;
}

bool DistanceSensor::readRightEdge(uint8_t &value)
{
    if (m_initialized)
    {
        m_wire->beginTransmission(SENSOR_ADDRESS);
        m_wire->write(RIGHT_EDGE_ADDR);
        if (m_wire->endTransmission() != 0)
        {
            return false;
        }
        
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
        if (1 <= m_wire->available())
        {
            value = m_wire->read();
            return true;
        }
    
    
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
        if (1 <= m_wire->available())
        {
            value = m_wire->read();
            return true;
        }
    }
    
    return false;
}

bool DistanceSensor::readPeakEdge(uint8_t &value)
{
    if (m_initialized)
    {
        m_wire->beginTransmission(SENSOR_ADDRESS);
        m_wire->write(PEAK_EDGE_ADDR);
        if (m_wire->endTransmission() != 0)
        {
            return false;
        }
        
        m_wire->requestFrom(SENSOR_ADDRESS, (uint8_t)1);
        if (1 <= m_wire->available())
        {
            value = m_wire->read();
            return true;
        }
    }
    
    return false;
}