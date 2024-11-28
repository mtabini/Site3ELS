#pragma once

#include <Arduino.h>

#include <Config.hpp>

#include <Encoder.hpp>
#include <Leadscrew.hpp>
#include <Display.hpp>
#include <Tachometer.hpp>


typedef struct {
#ifdef SIMULATE_ENCODER
  EncoderSimulator &encoder;
#else // SIMULATE_ENCODER
  Encoder &encoder;
#endif // SIMULATE_ENCODER
  Leadscrew &leadscrew;
  Display &display;
  Tachometer &tachometer;

  uint32_t updateInterval;
} SerialDebugSetup;


class SerialDebug {
  friend bool _serialDebugTimerCallback(repeating_timer_t *rt);
public:
  SerialDebug(SerialDebugSetup setup) : _setup(setup) {}

  void begin();

protected:
  SerialDebugSetup _setup;
  repeating_timer_t _timer;
  critical_section_t _cs;

  void _loop();
};

