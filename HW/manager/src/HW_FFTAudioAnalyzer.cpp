// HW_FFTAudioAnalyzer.cpp
#include "HW_FFTAudioAnalyzer.h"
#include "PlatformConstants.h"

bool HW_FFTAudioAnalyzer::begin(void) 
{
    bool success = m_microphone.begin();
    return FFTAudioAnalyzer::begin();
}

void HW_FFTAudioAnalyzer::getMagnitudes(float* outMagnitudes) 
{
    m_microphone.readSamples(m_rawSamples, FFT_SAMPLES);

    static const float NORMALIZE_24BIT = 1.0f / 8388608.0f;
    for (size_t idx = 0; idx < FFT_SAMPLES; idx++) 
    {
        m_realComponent[idx] = static_cast<float>(m_rawSamples[idx]) * NORMALIZE_24BIT;
        m_imagComponent[idx] = 0.0f;
        
        float multiplier = 0.5f * (1.0f - cos(2.0f * M_PI * idx / (FFT_SAMPLES - 1)));
        m_realComponent[idx] *= multiplier;
    }

    m_FFT.compute(m_realComponent, m_imagComponent, FFT_SAMPLES, FFT_FORWARD);
    m_FFT.complexToMagnitude(m_realComponent, m_imagComponent, FFT_SAMPLES);

    size_t lowIdx = 0;
    size_t midIdx = NUM_LOW_BINS;
    size_t highIdx = NUM_LOW_BINS + NUM_MID_BINS;
    for (size_t idx = 0; idx < NUM_LOW_BINS; idx++)
    {
        outMagnitudes[idx] = m_realComponent[LOW_BINS[idx]];
    }
    for (size_t idx = 0; idx < NUM_MID_BINS; idx++)
    {
        outMagnitudes[midIdx+idx] = m_realComponent[MID_BINS[idx]];
    }
    for (size_t idx = 0; idx < NUM_HIGH_BINS; idx++)
    {
        outMagnitudes[highIdx+idx] = m_realComponent[HIGH_BINS[idx]];
    }
}

void HW_FFTAudioAnalyzer::getMaxMagnitudes(float* maxMagnitudes)
{
    // Copy low frequencies into first section
    memcpy(maxMagnitudes, HW::LOW_MAX_MAGNITUDES, NUM_LOW_BINS * sizeof(float));
    
    // Copy mid frequencies starting at index NUM_LOW_BINS
    memcpy(maxMagnitudes + NUM_LOW_BINS, HW::MID_MAX_MAGNITUDES, NUM_MID_BINS * sizeof(float));
    
    // Copy high frequencies starting at index NUM_LOW_BINS + NUM_MID_BINS
    memcpy(maxMagnitudes + NUM_LOW_BINS + NUM_MID_BINS, HW::HIGH_MAX_MAGNITUDES, NUM_HIGH_BINS * sizeof(float));
}