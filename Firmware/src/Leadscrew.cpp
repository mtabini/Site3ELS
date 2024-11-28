#include <leadscrew.hpp>

void Leadscrew::begin() {
  critical_section_init_with_lock_num(&_cs, 10);
}

void Leadscrew::loop() {
  LeadscrewState state;

  while(1) {
    // Read the current state variables and
    // write the spindle speed

    critical_section_enter_blocking(&_cs);
    state = _guardedState;
    _watchdog++;
    critical_section_exit(&_cs);

    // If the leadscrew is not engaged, do nothing

    if (!state.engaged) {
      continue;
    }

    // Calculate the next desired position based on the
    // number of steps recorded by the encoder and the
    // ratio of the leadscrew to the spindle

    _stepper.enabled(true);
    _stepper.desiredPosition = _stepper.desiredPosition + float(_encoder.positionDifference()) * state.spindleToLeadScrewRatio;

    // Loop the stepper; this moves the motor if needed

    _stepper.loop();
  }
}

void Leadscrew::engage(bool engage) {
  critical_section_enter_blocking(&_cs);
  _guardedState.engaged = engage;
  _stepper.desiredPosition = _stepper.position;
  critical_section_exit(&_cs);

  _encoder.positionDifference(); // Clear the position difference
  _state.engaged = engage;
}

bool Leadscrew::engaged() {
  return _state.engaged;
}

void Leadscrew::powerFeedIPR(float feedRate) {
  // Calculate the lead screw to spindle ratio based on the feed rate,
  // the leadscrew pitch, the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.
  //
  // The ratio is given by:
  // LS pitch / feed rate *                // number of LS revolutions per spindle revolution
  // LS reduction factor *                 // multiplied by the reduction factor between the input shaft of the LS and the screw
  // stepper res / encoder res             // multiplied by the ratio of steps per revolution of the encoder to the steps per revolution of the stepper

  float spindleToLeadScrewRatio = feedRate * _setup.leadscrewPitch *
                                  _setup.leadScrewReductionFactor * 
                                  _stepper.stepsPerRevolution() / _encoder.stepsPerRevolution();

  _state.spindleToLeadScrewRatio = spindleToLeadScrewRatio;

  critical_section_enter_blocking(&_cs);
  _guardedState.spindleToLeadScrewRatio = spindleToLeadScrewRatio;
  critical_section_exit(&_cs);
}

float Leadscrew::powerFeedIPR() {
  // Back-calculate the feed rate based on the lead screw to spindle ratio,
  // the leadscrew pitch, the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.

  return _state.spindleToLeadScrewRatio * _encoder.stepsPerRevolution() / 
          (_setup.leadscrewPitch * _setup.leadScrewReductionFactor * _stepper.stepsPerRevolution());
}

void Leadscrew::threadTPI(uint8_t tpi) {
  // Calculate the lead screw to spindle ratio based on the TPI,
  // the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.
  //
  // The ratio is given by:
  //
  // LS pitch / TPI *                       // number of LS revolutions per spindle revolution
  // LS reduction factor *                  // multiplied by the reduction factor between the input shaft of the LS and the screw
  // stepper res / encoder res              // multiplied by the ratio of steps per revolution of the encoder to the steps per revolution of the stepper

  float spindleToLeadScrewRatio = _setup.leadscrewPitch / float(tpi) *
                                  _setup.leadScrewReductionFactor * 
                                  _stepper.stepsPerRevolution() / _encoder.stepsPerRevolution();

  _state.spindleToLeadScrewRatio = spindleToLeadScrewRatio;

  critical_section_enter_blocking(&_cs);
  _guardedState.spindleToLeadScrewRatio = spindleToLeadScrewRatio;
  critical_section_exit(&_cs);
}

float Leadscrew::threadTPI() {
  // Back-calculate the TPI based on the lead screw to spindle ratio,
  // the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.

  return _setup.leadscrewPitch * _setup.leadScrewReductionFactor * _stepper.stepsPerRevolution() / 
        (_state.spindleToLeadScrewRatio * _encoder.stepsPerRevolution());
}

void Leadscrew::threadMetric(float pitch) {
  // Calculate the lead screw to spindle ratio based on the pitch,
  // the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.

  const float tpi = 25.4 / pitch;
  const float ratio = _setup.leadscrewPitch / tpi;
  const float reducedRatio = ratio * _setup.leadScrewReductionFactor;
  const float spindleToLeadScrewRatio = reducedRatio * _stepper.stepsPerRevolution() / _encoder.stepsPerRevolution();

  _state.spindleToLeadScrewRatio = spindleToLeadScrewRatio;

  critical_section_enter_blocking(&_cs);
  _guardedState.spindleToLeadScrewRatio = spindleToLeadScrewRatio;
  critical_section_exit(&_cs);
}

float Leadscrew::threadMetric() {
  // Back-calculate the pitch based on the lead screw to spindle ratio,
  // the lead screw reduction factor,
  // and the ratio of steps per revolution of the encoder to the
  // steps per revolution of the stepper.

  const float tpi = _setup.leadscrewPitch * _setup.leadScrewReductionFactor * _stepper.stepsPerRevolution() / 
                    (_state.spindleToLeadScrewRatio * _encoder.stepsPerRevolution());

  return 25.4 / tpi;
}


