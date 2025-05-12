#pragma once

#include <cstddef>
#include <cstdint>
static constexpr float NORMALIZED_MIN = 0.0f;
static constexpr float NORMALIZED_MAX = 1.0f;
static constexpr float CONNECTION_TIMEOUT = 1.0f;
static constexpr float HEARTBEAT_TIMEOUT = 1.0f;
static constexpr float I2C_WRITE_TIMEOUT = 0.005f;
static constexpr float PASSIVE_BRIGHTNESS_TIMEOUT = 60.0f;
static constexpr float AUDIO_REACTIVITY_TIMEOUT = 5.0f;
static constexpr float MANAGER_STATUS_TIMEOUT = 0.2f;
static constexpr float MANUAL_MODE_TIMEOUT = 5.0f;

static constexpr uint16_t DISTANCE_OVERRIDE_THRESHOLD = 4000;
static constexpr float DISTANCE_OVERRIDE_TIMEOUT = 3.0f;

static constexpr float TRANSITION_DURATION = 0.5f;

static constexpr float UINT16_MAX_FLOAT = 65535.0f;
static constexpr float INT24_MAX_FLOAT = 8388607.0f;

static constexpr float MIN_BRIGHTNESS = 0.2f;

static constexpr size_t MAC_ADDRESS_SIZE = 6;

static constexpr size_t NUM_AUDIO_BUCKETS = 3;
static constexpr size_t NUM_LOW_BINS = 5; 
static constexpr size_t NUM_MID_BINS = 5; 
static constexpr size_t NUM_HIGH_BINS = 5; 
static constexpr size_t NUM_TOTAL_BINS = NUM_LOW_BINS + NUM_MID_BINS + NUM_HIGH_BINS;
static constexpr float AUDIO_DETECTION_THRESHOLD = 0.25f;
static constexpr size_t SAMPLE_RATE = 48000;
static constexpr size_t FFT_SAMPLES = 1024;
static constexpr float LOW_FREQUENCIES[NUM_LOW_BINS] = {
    46.875f,    // Bin 1
    93.75f,     // Bin 2
    140.625f,   // Bin 3
    187.5f,     // Bin 4
    234.375f,   // Bin 5
};
static constexpr float MID_FREQUENCIES[NUM_MID_BINS] = {
    937.5f,     // Bin 20 (around 940Hz)
    1406.25f,   // Bin 30 (around 1.4kHz)
    1875.0f,    // Bin 40 (around 1.9kHz)
    2343.75f,   // Bin 50 (around 2.3kHz)    
    2812.5f,    // Bin 60 (around 2.8kHz)
};
static constexpr float HIGH_FREQUENCIES[NUM_HIGH_BINS] = {
    3750.0f,    // Bin 80 (around 3.8kHz)
    4218.75f,   // Bin 90 (around 4.2kHz)
    4687.5f,    // Bin 100 (around 4.7kHz)
    5156.25f,   // Bin 110 (around 5.2kHz)
    5625.0f,    // Bin 120 (around 5.6kHz)
};
static constexpr size_t LOW_BINS[NUM_LOW_BINS] = {
    1,  // 46.875f, (kick drum fundamental)
    2,  // 93.75f,   (upper kick/lower bass)
    3,  // 140.625f, (upper kick/lower bass)
    4,  // 187.5f,  (bass guitar/synth)
    5   // 234.375f (upper harmonics)
};
static constexpr size_t MID_BINS[NUM_MID_BINS] = {
    20,  // 940Hz
    30,  // 1.4kHz
    40,  // 1.9kHz
    50,  // 2.3kHz
    60   // 2.8kHz
};
static constexpr size_t HIGH_BINS[NUM_HIGH_BINS] = {
    80,  // 3.8kHz
    90,  // 4.2kHz
    100, // 4.7kHz
    110, // 5.2kHz
    120  // 5.6kHz
};

// Hardware-specific constants
namespace HW
{
    // Voltage
    static constexpr float BIT_RESOLUTION_12 = 4095.0f;
    static constexpr float AREF = 3.3f;
    static constexpr float DAC_PWR_V = 3.3f;
    static constexpr float DAC_MAX_V = 1.6f;
    static constexpr float PWR_MIN_V = 0.1f;
    static constexpr float PWR_MAX_V = 12.0f;
    static constexpr float DEFAULT_V = 0.1f;

    static constexpr uint8_t DEBOUNCE_THRESHOLD = 100;
    
    // RMT configuration
    static constexpr uint32_t RMT_CLK_DIV = 80;      // RMT clock divider (80MHz / 80 = 1MHz)
    static constexpr uint32_t TOTAL_PERIOD_US = 10000;
    
    // Stool distance sensing
    static constexpr float DISTANCE_DETECTED_THRESHOLD_V = 1.0f;

    static constexpr float MAX_BRIGHTNESS_NORMALIZED = 1.0f;
    static constexpr float MIN_BRIGHTNESS_NORMALIZED = 0.0f;

    static constexpr float LOW_MAX_MAGNITUDES[NUM_LOW_BINS] =
    {
        10.0f,
        10.0f,
        10.0f,
        10.0f,
        10.0f 
    };
    static constexpr float MID_MAX_MAGNITUDES[NUM_MID_BINS] =
    {
        10.0f,
        10.0f,
        10.0f,
        10.0f,
        10.0f 
    };
    static constexpr float HIGH_MAX_MAGNITUDES[NUM_HIGH_BINS] =
    {
        10.0f,
        10.0f,
        10.0f,
        10.0f,
        10.0f 
    };

    static constexpr size_t EEPROM_SIZE = 512;
    static constexpr const char* NVS_NAMESPACE = "neon_mgr";
}

// Unreal Engine specific constants
namespace UE
{
    static constexpr float MIN_SPEED = 0.0f;
    static constexpr float MAX_SPEED = 75.0f;

    // Camera distance (cm)
    static constexpr float ANGLE_WINDOW = 30.0f;
    static constexpr float INVALID_DISTANCE = 0.0f;
    static constexpr float MIN_DISTANCE = 0.4f;
    static constexpr float MAX_DISTANCE = 3.0f;
    static constexpr float DEFAULT_DISTANCE = 100.0f;

    static constexpr float MAX_BRIGHTNESS = 5.0f;
    static constexpr float MIN_BRIGHTNESS = 0.0f;

    static constexpr float LOW_MAX_MAGNITUDES[NUM_LOW_BINS] =
    {
        75.0f,
        75.0f,
        75.0f,
        75.0f,
        75.0f 
    };
    static constexpr float MID_MAX_MAGNITUDES[NUM_MID_BINS] =
    {
        75.0f,
        75.0f,
        75.0f,
        75.0f,
        75.0f 
    };
    static constexpr float HIGH_MAX_MAGNITUDES[NUM_HIGH_BINS] =
    {
        75.0f,
        75.0f,
        75.0f,
        75.0f,
        75.0f 
    };
}