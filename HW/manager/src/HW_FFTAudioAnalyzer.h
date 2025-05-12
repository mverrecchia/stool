// HW_FFTAudioAnalyzer.h
#pragma once

#include "FFTAudioAnalyzer.h"
#include "MicrophoneSensor.h"
#include "PlatformUtils.h"
#include "PlatformConstants.h"
#include "arduinoFFT.h"

class HW_FFTAudioAnalyzer : public FFTAudioAnalyzer {
public:
   HW_FFTAudioAnalyzer(PlatformUtils* utils)
    : FFTAudioAnalyzer(utils)
    , m_microphone()
    , m_FFT(m_realComponent, m_imagComponent, FFT_SAMPLES, SAMPLE_RATE)
   {}

   virtual ~HW_FFTAudioAnalyzer() = default;  // Virtual destructor

   bool begin(void) override;

protected:
   void getMagnitudes(float* outMagnitudes) override;
   void getMaxMagnitudes(float* maxMagnitudes) override;

private:
   // Buffers for FFT
   int32_t m_rawSamples[FFT_SAMPLES];
   float m_realComponent[FFT_SAMPLES];
   float m_imagComponent[FFT_SAMPLES];

   MicrophoneSensor m_microphone;
   ArduinoFFT<float> m_FFT;
};