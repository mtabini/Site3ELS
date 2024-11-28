#pragma once

#include <Arduino.h>
#include <Stepper.hpp>
#include <Encoder.hpp>


struct LeadscrewSetup {
  Stepper &stepper;
  Encoder &encoder;

  uint8_t leadscrewPitch;             // Pitch of the leadscrew in TPI
  uint8_t leadScrewReductionFactor;   // Reduction factor of the leadscrew
};


struct LeadscrewState {
  bool engaged = false;
  float spindleToLeadScrewRatio = 1.0;
};


class Leadscrew {
  public:
    Leadscrew(LeadscrewSetup setup) : _setup(setup), _stepper(setup.stepper), _encoder(setup.encoder) {}

    void begin();
    void loop();
    
    void engage(bool engage);
    bool engaged();

    void powerFeedIPR(float feedRate);
    float powerFeedIPR();

    void threadTPI(uint8_t tpi);
    float threadTPI();

    void threadMetric(float pitch);
    float threadMetric();

    inline uint32_t watchdog() {
      uint32_t result;

      critical_section_enter_blocking(&_cs);
      result = _watchdog;
      critical_section_exit(&_cs);

      return result;
    }

  protected:
    LeadscrewSetup _setup;
    Stepper _stepper;
    Encoder &_encoder;
    critical_section_t _cs;

    /**
     * @brief This variable provides a rudimentary
     *        watchdog. It is incremented in the loop
     *        and can be observed to determine if the
     *        loop is running.
     * 
     */
    uint32_t _watchdog = 0; //TODO implement safety stop

    /**
     * @brief This variable holds a copy of the leadscrew's
     *       state that can be accessed without a critical
     *       section.
     * 
     */
    LeadscrewState _state;

    /**
     * @brief This variable holder the leadscrew's “working“
     *        state that is protected by a critical section.
     * 
     */
    LeadscrewState _guardedState;
};

