#include <Arduino.h>
#include <SPI.h>

#include <Config.hpp>

#include <Leadscrew.hpp>
#include <Encoder.hpp>
#include <Stepper.hpp>
#include <Tachometer.hpp>
#include <Display.hpp>
#include <SerialDebug.hpp>


Stepper stepper({
  Config.StepperDirectionPin,
  Config.StepperPulsePin,
  Config.StepperStepsPerRevolution,
});

#ifdef SIMULATE_ENCODER
EncoderSimulator encoder({
#else // SIMULATE_ENCODER
Encoder encoder({
#endif // SIMULATE_ENCODER
  Config.EncoderMOSIPin,
  Config.EncoderClkPin,
  Config.EncoderClockSpeed,
  Config.EncoderResolutionBits,
  Config.EncoderStepsPerRevolution,
  Config.EncoderUpdateInterval,
});

Leadscrew leadScrew({
  stepper,
  encoder,
  Config.LeadScrewPitch,
  Config.LeadScrewReductionFactor,
});

Tachometer tachometer(encoder);

Display display({
  Config.DisplayClkPin,
  Config.DisplayDioPin,
  Config.DisplayStbPin,
  Config.DisplayTPIThreads,
  Config.DisplayMetricThreads,
  Config.DisplayDefaultTPIThreadIndex,
  Config.DisplayDefaultMetricThreadIndex,
  Config.DisplayMinPowerFeedIPR,
  Config.DisplayMaxPowerFeedIPR,
  Config.DisplayDefaultPowerFeedIPR,
  leadScrew,
  tachometer,
  Config.DisplayBanner,
  Config.DisplayUpdateInterval,
  Config.DisplayLoopInterval,
  Config.DisplayButtonQueueSize,
  Config.DisplayAutoOffInterval,
});

SerialDebug serialDebug({
  encoder,
  leadScrew,
  display,
  tachometer,
  Config.SerialDebugUpdateInterval,
});

void setup1() {
  encoder.begin();
  leadScrew.begin();
  leadScrew.loop();
}

void setup() {
  stepper.begin();
  tachometer.begin();
  display.begin();
  // Serial.begin(115200);
  // serialDebug.begin();
}

void loop() {
}