#pragma once

#include <Arduino.h>

struct StepperSetup {
  uint8_t directionPin;
  uint8_t pulsePin;
  float stepsPerRevolution;
};

class Stepper {
public:
  float position = 0;
  float desiredPosition = 0;

  Stepper(StepperSetup setup) : _setup(setup) {}
  
  void begin();
  void loop();

  bool enabled();
  void enabled(bool enabled);

  inline float stepsPerRevolution() {
    return _setup.stepsPerRevolution;
  }

protected:
  StepperSetup _setup;

  bool _enabled = false;
  bool _stepping = false;
};

