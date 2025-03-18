// FFTAudioAnalyzer.h
#pragma once
#include <vector>
#include <cstdio>    // For snprintf
#include <cstring>
#include <algorithm> // For min/max
#include "PlatformUtils.h"
#include "PlatformConstants.h"

class FFTAudioAnalyzer {
public:
    FFTAudioAnalyzer(PlatformUtils* utils) 
        : m_pUtils(utils)
    {}
    virtual ~FFTAudioAnalyzer() = default;

    void setFastAlpha(float alpha) { m_fastAlpha = alpha; }
    void setSlowAlpha(float alpha) { m_slowAlpha = alpha; }
    void setNormalizedMagnitudes(float* magnitudes) { memcpy(m_normalizedMagnitudes, magnitudes, NUM_TOTAL_BINS * sizeof(float)); }
    float* getNormalizedMagnitudes() { return m_normalizedMagnitudes; }

    // Main processing
    virtual void update(float deltaTime);
    virtual bool begin(void);

    float LOW_WEIGHTS[NUM_LOW_BINS] = {
        0.4f,  // 46.875f
        0.4f,  // 93.75f 
        0.1f,  // 140.625f
        0.1f,  // 187.5f 
        0.0f   // 234.375f
    };
    float MID_WEIGHTS[NUM_MID_BINS] = {
        0.2f,  // 281.25Hz
        0.2f,  // 468.75Hz
        0.2f,  // 937.5Hz
        0.2f,  // 1875Hz
        0.2f   // 3750Hz
    };
    float HIGH_WEIGHTS[NUM_HIGH_BINS] = {
        0.5f,  // 4687.5Hz
        0.5f,  // 7031.25Hz
        0.0f,  // 9375Hz
        0.0f,  // 14062.5Hz
        0.0f   // 18750Hz
    };

protected:
    PlatformUtils* m_pUtils;
    float m_fastAlpha = 0.9f;
    float m_slowAlpha = 0.2f;

    float m_normalizedMagnitudes[NUM_TOTAL_BINS] = {0.0f};
    float m_magnitudes[NUM_TOTAL_BINS] = {0.0f};
    float m_prevMagnitudes[NUM_TOTAL_BINS] = {0.0f};
    float m_maxMagnitudes[NUM_TOTAL_BINS] = {0.0f};

    void outputDebugInfo(float* outMagnitudes);

    virtual void getMagnitudes(float* outMagnitudes) = 0;
    virtual void getMaxMagnitudes(float* maxMagnitudes) = 0;
};