// HW_FFTAudioAnalyzer.cpp
#include "HW_FFTAudioAnalyzer.h"
#include "PlatformTypes.h"
#include "PlatformConstants.h"
#include <cmath>

bool HW_FFTAudioAnalyzer::begin(void)
{
    FFTAudioAnalyzer::begin();
    return m_microphone.begin();
}

void HW_FFTAudioAnalyzer::getMagnitudes(float* outMagnitudes)
{
    m_microphone.readSamples(m_rawSamples, FFT_SAMPLES);

    static const float NORMALIZE_24BIT = 1.0f / 8388608.0f;
    for (size_t idx = 0; idx < FFT_SAMPLES; idx++) 
    {
        m_realComponent[idx] = (static_cast<float>(m_rawSamples[idx])) * NORMALIZE_24BIT;
        m_imagComponent[idx] = 0.0f;
        
        // Comment out Hann window for now - this didn't look entirely helpful
        // float windowValue = 0.5f * (1.0f - cosf(2.0f * PI * idx / (FFT_SAMPLES - 1)));
        // m_realComponent[idx] *= windowValue;
    }

    m_FFT.compute(FFT_FORWARD);
    m_FFT.complexToMagnitude(m_realComponent, m_imagComponent, FFT_SAMPLES);

    // Extract specific frequency bins
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
    memcpy(maxMagnitudes, HW::LOW_MAX_MAGNITUDES, NUM_LOW_BINS * sizeof(float));
    memcpy(maxMagnitudes + NUM_LOW_BINS, HW::MID_MAX_MAGNITUDES, NUM_MID_BINS * sizeof(float));
    memcpy(maxMagnitudes + NUM_LOW_BINS + NUM_MID_BINS, HW::HIGH_MAX_MAGNITUDES, NUM_HIGH_BINS * sizeof(float));
}