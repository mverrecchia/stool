#include "FFTAudioAnalyzer.h"
#include "PlatformConstants.h"

static constexpr size_t lowIdx = 0;
static constexpr size_t midIdx = NUM_LOW_BINS;
static constexpr size_t highIdx = NUM_LOW_BINS + NUM_MID_BINS;

bool FFTAudioAnalyzer::begin(void) 
{
    getMaxMagnitudes(m_maxMagnitudes);

    return true;
}

void FFTAudioAnalyzer::update(float deltaTime) 
{
    getMagnitudes(m_magnitudes);

    for (size_t idx = 0; idx < NUM_TOTAL_BINS; idx++)
    {
        float magnitude = m_magnitudes[idx];
        float alpha = (magnitude > m_prevMagnitudes[idx]) ? m_fastAlpha : m_slowAlpha;
        m_magnitudes[idx] = alpha * magnitude + (1.0f - alpha) * m_prevMagnitudes[idx];
        m_prevMagnitudes[idx] = m_magnitudes[idx];
    }

    for (size_t idx = 0; idx < NUM_TOTAL_BINS; idx++)
    {
        m_magnitudes[idx] = m_magnitudes[idx] / m_maxMagnitudes[idx];
        m_magnitudes[idx] = m_pUtils->clamp(m_magnitudes[idx], NORMALIZED_MIN, NORMALIZED_MAX);
    }
    // debug for now
    // outputDebugInfo(m_magnitudes);

    setNormalizedMagnitudes(m_magnitudes);
}

void FFTAudioAnalyzer::outputDebugInfo(float* outMagnitudes)
{
    char magLine[250];
    snprintf(magLine, sizeof(magLine),
            "Bins: %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f] %.2fHz[%.4f]",

            LOW_FREQUENCIES[0],  outMagnitudes[0],
            LOW_FREQUENCIES[1],  outMagnitudes[1],
            LOW_FREQUENCIES[2],  outMagnitudes[2],
            LOW_FREQUENCIES[3],  outMagnitudes[3],
            LOW_FREQUENCIES[4],  outMagnitudes[4],
            MID_FREQUENCIES[0],  outMagnitudes[5],
            MID_FREQUENCIES[1],  outMagnitudes[6],
            MID_FREQUENCIES[2],  outMagnitudes[7],
            MID_FREQUENCIES[3],  outMagnitudes[8],
            MID_FREQUENCIES[4],  outMagnitudes[9],
            HIGH_FREQUENCIES[0], outMagnitudes[10],
            HIGH_FREQUENCIES[1], outMagnitudes[11],
            HIGH_FREQUENCIES[2], outMagnitudes[12],
            HIGH_FREQUENCIES[3], outMagnitudes[13],
            HIGH_FREQUENCIES[4], outMagnitudes[14]);
    m_pUtils->logWarning(magLine);
}
