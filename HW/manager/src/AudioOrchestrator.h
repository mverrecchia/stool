#pragma once

#include "MessageTypes.h"
#include "PlatformTypes.h"
#include "PlatformUtils.h"
#include "PlatformConstants.h"
#include <cstdio>

class AudioOrchestrator {
public:
    AudioOrchestrator(PlatformUtils* utils)
        : m_pUtils(utils)
    {
        m_audioConfig = {
            .mode = AudioAssignmentMode_E::FIXED,
            .allowMultipleActive = false,
            .frequencyFlags = {FrequencyBand_E::LOW_FREQ},
            .lowFrequencyWeights = {0.4},
            .midFrequencyWeights = {0.4},
            .highFrequencyWeights = {0.4},
            .magnitudeThresholds = {AUDIO_DETECTION_THRESHOLD}
        };
        
        setAudioConfig(m_audioConfig);
    }
    
    void setAudioConfig(AudioConfiguration_S audioConfig) { m_audioConfig = audioConfig; }
    void generatePulse(float* magnitudes);
    bool getPulseReady() const { return m_pulseReady; }
    const AudioMessage_S* getAudioMessages(void) const { return m_audioMessages; }

private:
    PlatformUtils* m_pUtils;
    bool m_pulseReady = false;
    bool m_wasAboveThreshold = false;
    bool m_readyForNextPulse = true;
    float m_prevailingWeightedLowMagnitude = 0.0f;
    float m_prevailingWeightedMidMagnitude = 0.0f;
    float m_prevailingWeightedHighMagnitude = 0.0f;
    bool m_hasControllerPulsed[NUM_CONTROLLERS] = {false};
    float m_normalizedWeightedMagnitudes[NUM_TOTAL_BINS] = {0.0f};
    AudioConfiguration_S m_audioConfig;
    AudioMessage_S m_audioMessages[NUM_CONTROLLERS];
    
    bool generateFixedPulse(float lowMag, float midMag, float highMag);
    bool generateRandomPulse(float lowMag, float midMag, float highMag);
    bool generateSequentialPulse(float lowMag, float midMag, float highMag);
    bool getNewPulse(float lowMag, float midMag, float highMag);
    
    bool getMagnitudeAboveThreshold(FrequencyBand_E flag, float lowMag, float midMag, float highMag, float threshold);
};