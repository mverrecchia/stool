// shared/src/NeonManager.cpp
#include "NeonManager.h"
#include <cstddef>
#include <cstring>

void NeonManager::updateInputFlags(float deltaTime)
{
    manageDirectControlFlags();

    if (m_audioReactivityEnabled && m_pAudioOrchestrator->getPulseReady())
    {
        m_directControlAudio = true;
        m_audioTimer.reset();
    }
    else
    {
        if (m_audioTimer.hasElapsed(AUDIO_REACTIVITY_TIMEOUT))
        {
            m_directControlAudio = false;
        }
        else 
        {
            m_audioTimer.update(deltaTime);
        }
    }
}

void NeonManager::update(float deltaTime)
{
    updateInputFlags(deltaTime);
    updateModes(deltaTime);

    if (m_audioReactivityEnabled)
    {
        updateAudioListening(deltaTime);
    }

    if (m_directControlManual)
    {
        runManualMode(deltaTime);
    }
    else if (m_directControlAudio)
    {
        runAudioMode(deltaTime);
    }
    else if (m_directControlProfile)
    {
        runProfileMode(deltaTime);
    }
    else
    {
        runPassiveMode(deltaTime);
    }

    // TODO - re-enable with correct distance sensor, analog is too susceptible to noise
    // runDistanceOverride(deltaTime);

    applyNeonSettings();
}

void NeonManager::runManualMode(float deltaTime)
{
    for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
    {
        setSupplyEnable(idx, m_manualRequestSupplies[idx].enable);
        setSupplyBrightness(idx, m_manualRequestSupplies[idx].brightness);
    }
    m_directControlManual = false;  
}

void NeonManager::runPassiveMode(float deltaTime)
{
    // Handle default behavior
}

// Audio mode has three submodes - all, random, sequential. Only one message is sent per pulse regardless of mode
// In all mode, all controllers pulse at the same time.
// In random mode, a random subset of controllers pulse.
// In sequential mode, controllers pulse in sequence.
void NeonManager::runAudioMode(float deltaTime)
{    
    if (m_pAudioOrchestrator->getPulseReady())
    {
        for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
        {
            AudioMessage_S audioMsg = m_pAudioOrchestrator->getAudioMessages()[idx];
            for (size_t supplyIdx = 0; supplyIdx < NUM_SUPPLIES_PER_CONTROLLER; supplyIdx++)
            {
                if (audioMsg.frequencyFlags[supplyIdx] == 1)
                {
                    m_audioRequestSupplies[supplyIdx].brightness = audioMsg.prevailingWeightedLowMagnitude;
                    m_audioRequestSupplies[supplyIdx].enable = true;
                }
                else if (audioMsg.frequencyFlags[supplyIdx] == 2)
                {
                    m_audioRequestSupplies[supplyIdx].brightness = audioMsg.prevailingWeightedMidMagnitude;
                    m_audioRequestSupplies[supplyIdx].enable = true;
                }
                else if (audioMsg.frequencyFlags[supplyIdx] == 4)
                {
                    m_audioRequestSupplies[supplyIdx].brightness = audioMsg.prevailingWeightedHighMagnitude;
                    m_audioRequestSupplies[supplyIdx].enable = true;
                }
                else
                {
                    m_audioRequestSupplies[supplyIdx].brightness = MIN_BRIGHTNESS;
                    m_audioRequestSupplies[supplyIdx].enable = true;
                }
                setSupplyEnable(supplyIdx, m_audioRequestSupplies[supplyIdx].enable);
                setSupplyBrightness(supplyIdx, m_pUtils->clamp(m_audioRequestSupplies[supplyIdx].brightness, MIN_BRIGHTNESS, NORMALIZED_MAX));
            }
        }
    }
}

void NeonManager::runProfileMode(float deltaTime)
{
    if (m_profileExecutor.getProfileActive())
    {
        updateProfileExecutor(deltaTime, true);
    }

    for (size_t idx = 0; idx < NUM_CONTROLLERS; idx++)
    {
        if (m_profileRequestParameters[idx].enable)
        {
            startProfileExecutor(m_profileRequestParameters[idx]);
        }
        else if (m_profileRequestParameters[idx].stopProfile)
        {
            stopProfileExecutor();
            m_profileActive = false;
        }
    }
}

void NeonManager::updateAudioListening(float deltaTime)
{
    m_pFftAnalyzer->update(deltaTime);
    m_pAudioOrchestrator->generatePulse(m_pFftAnalyzer->getNormalizedMagnitudes());
}

void NeonManager::manageDirectControlFlags(void)
{
    if (m_directControlProfile)
    {
        static bool hasSeenProfileActivity = false;
        bool anyActive = m_profileActive;

        if (anyActive)
        {
            hasSeenProfileActivity = true;
        }
        else if (hasSeenProfileActivity)
        {
            m_directControlProfile = false;
            hasSeenProfileActivity = false;
        }
    }
}

void NeonManager::updateProfileExecutor(float deltaTime, bool brightnessEnable)
{
    float supply0Value, supply1Value;
    if (m_profileExecutor.getProfileActive())
    {
        if (!m_profileActive)
        {
            m_profileActive = true;
            for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
            {
                m_profileSupplies[idx].brightness = getSupplyBrightness(idx);
                m_profileSupplies[idx].enable = getSupplyEnable(idx);
            }
        }

        m_profileExecutor.updateProfileValues(deltaTime, supply0Value, supply1Value);

        if (brightnessEnable)
        {
            for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
            {
                setSupplyBrightness(idx, supply0Value);
            }
        }

        if (!m_profileExecutor.getProfileActive())
        {
            m_profileActive = false;

            for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
            {
                setSupplyBrightness(idx, m_profileSupplies[idx].brightness);
                setSupplyEnable(idx, m_profileSupplies[idx].enable);
            }
        }
    }
}

void NeonManager::updateAudioActivity(float deltaTime)
{
    if (m_directControlAudio)
    {
        m_audioActivityTimer.update(deltaTime);
        if (m_audioActivityTimer.hasElapsed(AUDIO_REACTIVITY_TIMEOUT))
        {
            m_directControlAudio = false;
        }
    }
    else
    {
        m_audioActivityTimer.stop();
    }
}

void NeonManager::runDistanceOverride(float deltaTime)
{
    m_distanceOverrideTimer.update(deltaTime);
    updateDistanceValue();

    float distance = getDistance();
    // m_pUtils->logFloat(distance);
    bool objectDetected = getDistanceUnderThreshold();

    if (objectDetected && !m_wasObjectDetected)
    {
        m_distanceOverrideState = DistanceOverrideState_E::RISING_EDGE;
        m_transitionTimer = 0.0f;
        
        for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
        {
            m_distanceOverrideSupplies[idx].enable = getSupplyEnable(idx);
            m_distanceOverrideSupplies[idx].brightness = getSupplyBrightness(idx);
            setSupplyEnable(idx, false);
        }
        
        m_isInDistanceOverride = true;
    } 
    else if (!objectDetected && m_wasObjectDetected && m_isInDistanceOverride)
    {
        m_distanceOverrideState = DistanceOverrideState_E::FALLING_EDGE;
        m_transitionTimer = 0.0f;
    }
    
    m_wasObjectDetected = objectDetected;

    switch (m_distanceOverrideState)
    {
        case DistanceOverrideState_E::RISING_EDGE:
            handleRisingEdge(deltaTime);
            break;
            
        case DistanceOverrideState_E::ACTIVE:
            handleActiveState(deltaTime);
            break;
            
        case DistanceOverrideState_E::FALLING_EDGE:
            handleFallingEdge(deltaTime);
            break;
            
        case DistanceOverrideState_E::INACTIVE:
            // Nothing to do in inactive state
            break;
    }
}

void NeonManager::handleRisingEdge(float deltaTime)
{
    m_transitionTimer += deltaTime;
    
    // Phase 1: Turn on bottom supply (index 0) and ramp up over 1 second
    if (m_transitionTimer <= 0.75f)
    {
        setSupplyEnable(1, true);
        
        float progress = m_transitionTimer / 1.0f;
        float brightness = m_pUtils->lerp(0.0f, 1.0f, progress);
        setSupplyBrightness(1, brightness);
    }
    // Phase 2: Turn on outer supply (index 1) and ramp up over 1 second, starting 0.5s after bottom supply
    else if (m_transitionTimer <= 1.5f)
    {
        setSupplyBrightness(1, 1.0f);
        
        if (m_transitionTimer >= 0.5f)
        {
            setSupplyEnable(0, true);
            
            float progress = (m_transitionTimer - 0.5f) / 1.0f;
            progress = std::min(progress, 1.0f);
            float brightness = m_pUtils->lerp(0.0f, 1.0f, progress);
            setSupplyBrightness(0, brightness);
        }
    }
    else
    {
        m_distanceOverrideState = DistanceOverrideState_E::ACTIVE;
        m_transitionTimer = 0.0f;
        
        for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
        {
            setSupplyEnable(idx, true);
            setSupplyBrightness(idx, 1.0f);
        }
    }
}

void NeonManager::handleActiveState(float deltaTime)
{
}

void NeonManager::handleFallingEdge(float deltaTime)
{
    m_transitionTimer += deltaTime;
    
    if (m_transitionTimer <= 0.5f)
    {
        setSupplyBrightness(1, 1.0f);
        
        float progress = m_transitionTimer / 1.0f;
        float brightness = m_pUtils->lerp(1.0f, 0.0f, progress);
        setSupplyBrightness(0, brightness);
        
        if (progress >= 1.0f)
        {
            setSupplyEnable(0, false);
        }
    }
    else if (m_transitionTimer <= 1.5f)
    {
        setSupplyEnable(0, false);
        
        float progress = (m_transitionTimer - 0.5f) / 1.0f;
        float brightness = m_pUtils->lerp(1.0f, 0.0f, progress);
        setSupplyBrightness(1, brightness);
        
        if (progress >= 1.0f)
        {
            setSupplyEnable(1, false);
        }
    }
    else
    {
        for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
        {
            setSupplyEnable(idx, m_distanceOverrideSupplies[idx].enable);
            setSupplyBrightness(idx, m_distanceOverrideSupplies[idx].brightness);
        }
        
        m_isInDistanceOverride = false;
        m_distanceOverrideState = DistanceOverrideState_E::INACTIVE;
    }
}

void NeonManager::updateModes(float deltaTime)
{
    // Determine current mode
    updateAudioActivity(deltaTime);
    bool inProfileMode = m_profileExecutor.getProfileActive();
    bool inAudioMode = m_directControlAudio;

    if (inProfileMode && !m_wasInProfileMode)
    {
        enterProfileMode();
    }
    else if (inAudioMode && !m_wasInAudioMode)
    {
        enterAudioMode();
    }

    m_wasInProfileMode = inProfileMode;
    m_wasInAudioMode = inAudioMode;
}

void NeonManager::enterProfileMode(void)
{
   for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
   {
        setSupplyEnable(idx, true);
   }
}

void NeonManager::enterAudioMode(void)
{
    for (size_t idx = 0; idx < NUM_SUPPLIES_PER_CONTROLLER; idx++)
    {
        setSupplyEnable(idx, true);
    }
}

void NeonManager::enterPassiveMode(void)
{
    // do nothing for this mode
    m_pUtils->logWarning("Entering passive mode");
}
