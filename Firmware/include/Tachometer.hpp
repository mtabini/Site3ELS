#pragma once

#include <Arduino.h>
#include <Encoder.hpp>


class Tachometer {
  friend bool _tachometerTimerCallback(repeating_timer_t *rt);
public:
  Tachometer(Encoder &_encoder) : _encoder(_encoder) {}

  void begin();

  inline float speed() {
    float result;

    critical_section_enter_blocking(&_cs);
    result = _speed;
    critical_section_exit(&_cs);

    return result;
  }

protected:
  Encoder &_encoder;
  critical_section_t _cs;

  repeating_timer_t _timer;

  int64_t _lastPosition;
  absolute_time_t _lastPositionReadTime;

  float _speed;
  float _speedHistory[10];

  void _loop();
};