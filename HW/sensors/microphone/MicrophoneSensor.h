// MicrophoneSensor.h
#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

class MicrophoneSensor {
public:
    MicrophoneSensor() = default;
    
    bool begin();
    void readSamples(int32_t* samples, size_t sampleCount);

private:
    bool initI2S();

    // I2S config
    static constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
    static constexpr size_t SAMPLE_RATE = 48000;
};