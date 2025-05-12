// shared/include/NeonManager.h

#include "FFTAudioAnalyzer.h"
#include "AudioOrchestrator.h"
#include "ProfileExecutor.h"
#include "PlatformUtils.h"
#include "PlatformTypes.h"
#include "PlatformTimer.h"
#include "MessageTypes.h"

#include <memory>
#include <cstddef>
#include <cstring>

class NeonManager {
public:
    NeonManager(PlatformUtils* utils)
        : m_pUtils(utils)
        , m_pFftAnalyzer(nullptr)
        , m_pAudioOrchestrator(new AudioOrchestrator(m_pUtils))
        , m_profileExecutor(m_pUtils)
    {
        m_audioTimer.start();
    }
    virtual ~NeonManager() = default;

    virtual bool initialize(void) = 0;
    virtual void update(float deltaTime);
    void updateInputFlags(float deltaTime);
    void updateAudioListening(float deltaTime);
    void updateAudioActivity(float deltaTime);

    FFTAudioAnalyzer* getFFTAnalyzer() const        { return m_pFftAnalyzer; }
    void setControllerProfileActive(bool active) { m_profileActive = active; }

    void setAudioConfig(AudioConfiguration_S audioConfig)
    {
        m_pAudioOrchestrator->setAudioConfig(audioConfig);
    }

    void setMacAddress(const uint8_t* macAddress) { memcpy(m_macAddress, macAddress, MAC_ADDRESS_SIZE); }

    bool getSupplyEnable(size_t idx) const        { return m_suppliesState[idx].enable; }
    void setSupplyEnable(size_t idx, bool enable) { m_suppliesState[idx].enable = enable; }

    float getSupplyBrightness(size_t idx) const            { return m_suppliesState[idx].brightness; }
    void setSupplyBrightness(size_t idx, float brightness) { m_suppliesState[idx].brightness = brightness; }

    void setDistance(uint16_t distance)       { m_distance = distance; }
    uint16_t getDistance(void) const          { return m_distance; }
    bool getDistanceUnderThreshold() const { return m_distance < DISTANCE_OVERRIDE_THRESHOLD; }

protected:
    void runManualMode(float deltaTime);
    void runProfileMode(float deltaTime);
    void runAudioMode(float deltaTime);
    void runPassiveMode(float deltaTime);

    void enterProfileMode(void);
    void enterAudioMode(void);
    void enterPassiveMode(void);
    void updateModes(float deltaTime);
    
    void manageDirectControlFlags(void);

    virtual void applyNeonSettings(void) = 0;

    virtual void updateDistanceValue(void) = 0;
    void runDistanceOverride(float deltaTime);

    void startProfileExecutor(const ProfileRequestParameters_S& profile)
    {
        m_profileExecutor.startProfile(profile);
    }

    void stopProfileExecutor(void)
    {
        m_profileExecutor.stopProfile();
    }

    void updateProfileExecutor(float deltaTime, bool brightnessEnable);

    void handleRisingEdge(float deltaTime);
    void handleActiveState(float deltaTime);
    void handleFallingEdge(float deltaTime);

    uint8_t m_macAddress[MAC_ADDRESS_SIZE];

    // Common state
    PlatformUtils* m_pUtils;
    FFTAudioAnalyzer* m_pFftAnalyzer;
    AudioOrchestrator* m_pAudioOrchestrator;
    ProfileExecutor m_profileExecutor;

    SupplyParameters_S m_suppliesState[NUM_SUPPLIES_PER_CONTROLLER];
    SupplyParameters_S m_manualRequestSupplies[NUM_SUPPLIES_PER_CONTROLLER];
    SupplyParameters_S m_audioRequestSupplies[NUM_SUPPLIES_PER_CONTROLLER];
    SupplyParameters_S m_distanceOverrideSupplies[NUM_SUPPLIES_PER_CONTROLLER];
    ProfileRequestParameters_S m_profileRequestParameters[NUM_CONTROLLERS];
    SupplyParameters_S m_profileSupplies[NUM_SUPPLIES_PER_CONTROLLER];

    //  These need cleanup
    float m_transitionTimer = 0.0f;
    bool m_isInDistanceOverride = false;

    uint16_t m_distance;
    bool m_profileActive = false;

    // Main state inputs
    bool m_audioReactivityEnabled = true;
    bool m_directControlAudio = false;
    bool m_directControlManual = false;
    bool m_directControlProfile = false;

    bool m_wasInProfileMode;
    bool m_wasInAudioMode;
    bool m_wasInDistanceOverride;

private:
    DistanceOverrideState_E m_distanceOverrideState = DistanceOverrideState_E::INACTIVE;
    bool m_wasObjectDetected = false;
    
    Timer m_stateTimer;
    Timer m_audioTimer;

    Timer m_distanceOverrideTimer;
    Timer m_audioActivityTimer;
    Timer m_manualModeTimer;

    bool m_pulseReady = false;
};