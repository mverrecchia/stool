#pragma once

#include "PlatformUtils.h"
#include <memory>

class ProfileExecutor {
public:
    ProfileExecutor(PlatformUtils* utils) 
        : m_pUtils(utils)
        , m_active(false)
        , m_elapsedTime(0.0f)
    {}

    void startProfile(const ProfileRequestParameters_S& profile)
    {
        m_currentProfile = profile;
        m_active = true;
    }

    void stopProfile()
    {
        m_active = false; 
        m_elapsedTime = 0.0f;
    }

    bool getProfileActive() const { return m_active; }

    void updateProfileValues(float deltaTime, float& supply0, float& supply1)
    {
        m_elapsedTime += deltaTime;

        if (getProfileActive())
        {
            float baseValue = calculateProfileValue();
            supply0 = baseValue * m_currentProfile.magnitude;
            supply1 = baseValue * m_currentProfile.magnitude;
        }
    }

private:
    float calculateProfileValue(void)
    {
        if (!m_active) return 0.0f;
        
        float profileValue = 0.0f;
        float frequency = m_currentProfile.frequency;
        float phaseOffset = m_currentProfile.phase;
        float scaledTime = (m_elapsedTime * frequency) + phaseOffset;
        float t = fmod(scaledTime, 1.0f);
        
        switch (m_currentProfile.type)
        {
            case ProfileType_E::COS:
                profileValue = 0.5f + 0.5f * cos(2 * M_PI * t);
                break;

            case ProfileType_E::EXPONENTIAL:
                profileValue = 1.0f - exp(-3.0f * t);
                break;

            case ProfileType_E::BOUNCE:
                profileValue = 1.0f - (1.0f - t) * (1.0f - t);
                break;

            case ProfileType_E::PULSE:
            {
                const float HEARTBEAT_PERIOD = 1.0f;
                const float FIRST_PEAK_START = 0.0f;
                const float FIRST_PEAK_END = 0.15f;
                const float SECOND_PEAK_START = 0.075f;
                const float SECOND_PEAK_END = 0.40f;
                
                const float FIRST_RISE_RATIO = 0.3f;
                const float FIRST_RISE_POWER = 1.5f;
                const float FIRST_FALL_POWER = 2.0f;
                
                const float SECOND_RISE_RATIO = 0.15f;
                const float SECOND_RISE_POWER = 1.2f;
                const float SECOND_FALL_POWER = 1.2f;

                const float FIRST_PULSE_HEIGHT = 1.0f;
                const float SECOND_PULSE_HEIGHT = 0.7f;
                const float MIN_PULSE_VALUE = 0.08f;
                
                float cyclePosition = fmod(t, HEARTBEAT_PERIOD) / HEARTBEAT_PERIOD;

                float firstPulseValue = 0.0f;
                float secondPulseValue = 0.0f;
                    
                if (cyclePosition >= FIRST_PEAK_START && cyclePosition < FIRST_PEAK_END)
                {
                    float phase = (cyclePosition - FIRST_PEAK_START) / (FIRST_PEAK_END - FIRST_PEAK_START);
                    
                    if (phase < FIRST_RISE_RATIO)
                    {
                        firstPulseValue = pow(phase / FIRST_RISE_RATIO, FIRST_RISE_POWER);
                    }
                    else
                    {
                        firstPulseValue = pow(1.0f - ((phase - FIRST_RISE_RATIO) / (1.0f - FIRST_RISE_RATIO)), FIRST_FALL_POWER);
                    }
                    firstPulseValue *= FIRST_PULSE_HEIGHT;
                }
                
                if (cyclePosition >= SECOND_PEAK_START && cyclePosition < SECOND_PEAK_END)\
                {
                    float phase = (cyclePosition - SECOND_PEAK_START) / (SECOND_PEAK_END - SECOND_PEAK_START);
                    
                    if (phase < SECOND_RISE_RATIO)
                    {
                        secondPulseValue = pow(phase / SECOND_RISE_RATIO, SECOND_RISE_POWER);
                    }
                    else
                    {
                        secondPulseValue = pow(1.0f - ((phase - SECOND_RISE_RATIO) / (1.0f - SECOND_RISE_RATIO)), SECOND_FALL_POWER);
                    }
                    secondPulseValue *= SECOND_PULSE_HEIGHT;
                }
                
                float pulseValue = fmax(firstPulseValue, secondPulseValue);
                profileValue = fmax(MIN_PULSE_VALUE, fmin(pulseValue, 1.0f));
                break;
            }

            case ProfileType_E::TRIANGLE:
            {
                profileValue = (t < 0.5f) ? (2.0f * t) : (2.0f * (1.0f - t));
                break;
            }

            case ProfileType_E::ELASTIC:
            {
                const float decay = 3.0f;
                const float oscillations = 3.0f;
                profileValue = 1.0f - (exp(-decay * t) * cos(2.0f * M_PI * oscillations * t));
                break;
            }

            case ProfileType_E::CASCADE:
            {
                const float bounceCount = 3.0f;
                profileValue = pow(1.0f - t, 2.0f) * sin(2.0f * M_PI * bounceCount * t);
                break;
            }
    
            case ProfileType_E::FLICKER:
            {
                const float base = 0.5f;
                const float variance = 0.8f;
                
                const float random = (float)rand() / RAND_MAX;
                const float highFreqNoise = sin(t * 50.0f + random * 10.0f);
                
                profileValue = base + (highFreqNoise * variance);
                break;
            }

            default:
                profileValue = 0.0f;
                break;
        }
        
        return m_pUtils->clamp(profileValue, 0.0f, 1.0f);
    }

    PlatformUtils* m_pUtils;
    ProfileRequestParameters_S m_currentProfile;
    bool m_active;
    float m_elapsedTime;
};