// MicrophoneSensor.cpp
#include "MicrophoneSensor.h"

bool MicrophoneSensor::begin()
{
    bool success = initI2S();
    return success;
}

bool MicrophoneSensor::initI2S()
{
    // Configure I2S for INMP441
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // INMP441 gives us 24-bit in 32-bit word
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // Mono mode
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_MIC_SCK,    // SCK
        .ws_io_num = PIN_MIC_WS,      // WS
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_MIC_SD     // SD (data)
    };

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    err = i2s_set_pin(I2S_PORT, &pin_config);

    return true;
}

void MicrophoneSensor::readSamples(int32_t* samples, size_t sampleCount)
{
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_PORT, 
                            (void*)samples,
                            sampleCount * sizeof(int32_t),
                            &bytesRead,
                            portMAX_DELAY);

    size_t actualSamplesRead = bytesRead / sizeof(int32_t);

    // The INMP441 sends 24-bit data in a 32-bit word
    // Shift the 24-bit value to the least significant bits
    for (size_t idx = 0; idx < actualSamplesRead; idx++)
    {
        samples[idx] >>= 8;
    }
}