#include "AudioOrchestrator.h"
#include "PlatformTypes.h"
#include <cstring>
#include <cstddef>
#include <cstdio>

void AudioOrchestrator::generatePulse(float* magnitudes)
{
    m_prevailingWeightedLowMagnitude = 0.0f;
    m_prevailingWeightedMidMagnitude = 0.0f;
    m_prevailingWeightedHighMagnitude = 0.0f;

    bool anyControllerActive = false;
    m_pulseReady = false;

    for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    {
        for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
        {
            m_audioMessages[idx].frequencyFlags[supplyIdx] = FrequencyBand_E::NONE_FREQ;
        }
        m_audioMessages[idx].prevailingWeightedLowMagnitude = 0.0f;
        m_audioMessages[idx].prevailingWeightedMidMagnitude = 0.0f;
        m_audioMessages[idx].prevailingWeightedHighMagnitude = 0.0f;
    }

    for (size_t idx = 0; idx < NUM_LOW_BINS; idx++)
    {
        m_normalizedWeightedMagnitudes[idx] = magnitudes[idx] * m_audioConfig.lowFrequencyWeights[idx];
        m_prevailingWeightedLowMagnitude = (m_prevailingWeightedLowMagnitude > m_normalizedWeightedMagnitudes[idx]) ? m_prevailingWeightedLowMagnitude : m_normalizedWeightedMagnitudes[idx];
    }
    for (size_t idx = 0; idx < NUM_MID_BINS; idx++)
    {
        m_normalizedWeightedMagnitudes[NUM_LOW_BINS + idx] = magnitudes[NUM_LOW_BINS + idx] * m_audioConfig.midFrequencyWeights[idx];
        m_prevailingWeightedMidMagnitude = (m_prevailingWeightedMidMagnitude > m_normalizedWeightedMagnitudes[NUM_LOW_BINS + idx]) ? m_prevailingWeightedMidMagnitude : m_normalizedWeightedMagnitudes[NUM_LOW_BINS + idx];
    }
    for (size_t idx = 0; idx < NUM_HIGH_BINS; idx++)
    {
        m_normalizedWeightedMagnitudes[NUM_LOW_BINS + NUM_MID_BINS + idx] = magnitudes[NUM_LOW_BINS + NUM_MID_BINS + idx] * m_audioConfig.highFrequencyWeights[idx];
        m_prevailingWeightedHighMagnitude = (m_prevailingWeightedHighMagnitude > m_normalizedWeightedMagnitudes[NUM_LOW_BINS + NUM_MID_BINS + idx]) ? m_prevailingWeightedHighMagnitude : m_normalizedWeightedMagnitudes[NUM_LOW_BINS + NUM_MID_BINS + idx];
    }

    switch(m_audioConfig.mode)
    {
        case AudioAssignmentMode_E::RANDOM:
            anyControllerActive = generateRandomPulse(m_prevailingWeightedLowMagnitude, m_prevailingWeightedMidMagnitude, m_prevailingWeightedHighMagnitude);
            break;
        case AudioAssignmentMode_E::SEQUENTIAL:
            anyControllerActive = generateSequentialPulse(m_prevailingWeightedLowMagnitude, m_prevailingWeightedMidMagnitude, m_prevailingWeightedHighMagnitude);
            break;
        default:
        case AudioAssignmentMode_E::FIXED:
            anyControllerActive = generateFixedPulse(m_prevailingWeightedLowMagnitude, m_prevailingWeightedMidMagnitude, m_prevailingWeightedHighMagnitude);
            break;
    }

    m_pulseReady = anyControllerActive;  // Only set ready if at least one controller should be active
}

bool AudioOrchestrator::getMagnitudeAboveThreshold(FrequencyBand_E flag, float lowMag, float midMag, float highMag, float threshold)
{
    return (lowMag > threshold) || (midMag > threshold) || (highMag > threshold);
}

bool AudioOrchestrator::generateFixedPulse(float lowMag, float midMag, float highMag)
{
    bool anyActive = false;
    
    for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    {
        bool controllerActive = false;
        for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
        {
            FrequencyBand_E flag = m_audioConfig.frequencyFlags[idx][supplyIdx];
            if (flag == FrequencyBand_E::NONE_FREQ)
                continue;
            
            // Determine which threshold index to use based on the frequency band
            size_t thresholdIndex = 0; // Default to low
            if (flag == FrequencyBand_E::MID_FREQ)
                thresholdIndex = 1;
            else if (flag == FrequencyBand_E::HIGH_FREQ)
                thresholdIndex = 2;
            
            float threshold = m_audioConfig.magnitudeThresholds[thresholdIndex];
            
            bool isAboveThreshold = getMagnitudeAboveThreshold(
                flag,
                lowMag,
                midMag,
                highMag,
                threshold
            );
            
            if (isAboveThreshold)
            {
                m_audioMessages[idx].frequencyFlags[supplyIdx] = flag;
                controllerActive = true;
            }
            else
            {
                m_audioMessages[idx].frequencyFlags[supplyIdx] = FrequencyBand_E::NONE_FREQ;
            }
        }
        
        if (controllerActive)
        {
            m_audioMessages[idx].prevailingWeightedLowMagnitude = lowMag;
            m_audioMessages[idx].prevailingWeightedMidMagnitude = midMag;
            m_audioMessages[idx].prevailingWeightedHighMagnitude = highMag;            
            anyActive = true;
        }
    }
    
    return anyActive;
}

bool AudioOrchestrator::generateSequentialPulse(float lowMag, float midMag, float highMag)
{
    // bool anyActive = false;
    // static size_t currentIdx = 0;

    // for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    // {
    //     for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
    //     {
    //         m_audioMessages[idx].frequencyFlags[supplyIdx] = FrequencyBand_E::NONE_FREQ;
    //     }
    // }

    // if (getNewPulse(lowMag, midMag, highMag))
    // {
    //     if (m_hasControllerPulsed[currentIdx])
    //     {
    //         currentIdx = (currentIdx + 1) % NUM_CONTROLLERS;
    //     }

    //     if (getMagnitudeAboveThreshold(m_audioConfig.frequencyFlags[currentIdx], lowMag, midMag, highMag, m_audioConfig.magnitudeThresholds[currentIdx]))
    //     {
    //         for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
    //         {
    //             m_audioMessages[currentIdx].frequencyFlags[supplyIdx] = m_audioConfig.frequencyFlags[currentIdx][supplyIdx];
    //         }
    //         m_hasControllerPulsed[currentIdx] = true;
    //         anyActive = true;
    //     }

    //     bool allPulsed = true;
    //     for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //     {
    //         if (!m_hasControllerPulsed[idx])
    //         {
    //             allPulsed = false;
    //             break;
    //         }
    //     }
    //     if (allPulsed)
    //     {
    //         m_readyForNextPulse = false;
    //         for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //         {
    //             m_hasControllerPulsed[idx] = false;
    //         }
    //     }
    // }

    // return anyActive;
    return false;
}

bool AudioOrchestrator::generateRandomPulse(float lowMag, float midMag, float highMag)
{
    // bool anyActive = false;

    // // Clear all flags
    // for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    // {
    //     m_audioMessages[idx].frequencyFlags = 0;
    // }

    // if (getNewPulse(lowMag, midMag, highMag))
    // {
    //     if (m_audioConfig.allowMultipleActive)
    //     {
    //         for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //         {
    //             if (!m_hasControllerPulsed[idx] && (rand() % 2))
    //             {  // 50% chance if not pulsed
    //                 if (getMagnitudeAboveThreshold(m_audioConfig.supplyFlags[idx], lowMag, midMag, highMag, m_audioConfig.magnitudeThreshold))
    //                 {
    //                     m_audioMessages[idx].frequencyFlags = m_audioConfig.supplyFlags[idx];
    //                     m_hasControllerPulsed[idx] = true;
    //                     anyActive = true;
    //                 }
    //             }
    //         }
    //     }
    //     else
    //     {
    //         // Find all eligible controllers that haven't pulsed yet
    //         size_t eligibleControllers[NUM_CONTROLLERS];
    //         size_t numEligible = 0;
            
    //         for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //         {
    //             if (!m_hasControllerPulsed[idx] && 
    //                 getMagnitudeAboveThreshold(m_audioConfig.supplyFlags[idx], 
    //                                         lowMag, midMag, highMag, 
    //                                         m_audioConfig.magnitudeThreshold))
    //             {
    //                 eligibleControllers[numEligible++] = idx;
    //             }
    //         }
            
    //         if (numEligible > 0)
    //         {
    //             size_t randomIdx = eligibleControllers[rand() % numEligible];
    //             m_audioMessages[randomIdx].frequencyFlags = m_audioConfig.supplyFlags[randomIdx];
    //             m_hasControllerPulsed[randomIdx] = true;
    //             anyActive = true;
    //         }
    //     }

    //     bool allPulsed = true;
    //     for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //     {
    //         if (!m_hasControllerPulsed[idx])
    //         {
    //             allPulsed = false;
    //             break;
    //         }
    //     }
    //     if (allPulsed)
    //     {
    //         m_readyForNextPulse = false;
    //         for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //         {
    //             m_hasControllerPulsed[idx] = false;
    //         }
    //     }
    // }

    // return anyActive;
    return false;
}

bool AudioOrchestrator::getNewPulse(float lowMag, float midMag, float highMag)
{
    // bool isAboveThreshold = false;
    
    // for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    // {
    //     if (getMagnitudeAboveThreshold(m_audioConfig.supplyFlags[idx], 
    //                                    lowMag,
    //                                    midMag,
    //                                    highMag, 
    //                                    m_audioConfig.magnitudeThreshold))
    //     {
    //         isAboveThreshold = true;
    //         break;
    //     }
    // }

    // bool isNewPulse = !m_wasAboveThreshold && isAboveThreshold && m_readyForNextPulse;
    
    // // If we've fallen below threshold, we can get ready for next pulse sequence
    // if (!isAboveThreshold && m_wasAboveThreshold)
    // {
    //     m_readyForNextPulse = true;
    //     for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    //     {
    //         m_hasControllerPulsed[idx] = false;
    //     }
    // }

    // m_wasAboveThreshold = isAboveThreshold;
    // return isNewPulse;
    return false;
}
