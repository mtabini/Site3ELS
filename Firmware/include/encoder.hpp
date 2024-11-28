#pragma once

#include <Arduino.h>
#include <SPI.h>


typedef struct {
  pin_size_t miso;
  pin_size_t clk;
  uint32_t clockSpeed;
  uint8_t resolutionBits;
  float stepsPerRevolution;
  uint32_t updateInterval;
} EncoderSetup;


class Encoder {
  friend bool _encoderTimerCallback(repeating_timer_t *rt);
public:
  Encoder(EncoderSetup setup) : _setup(setup) {}

  virtual void begin();

  inline int32_t positionDifference() {
    int32_t result;

    critical_section_enter_blocking(&_cs);
    result = _positionDifference;
    _positionDifference = 0;
    critical_section_exit(&_cs);

    return result;
  }

  inline float stepsPerRevolution() {
    return _setup.stepsPerRevolution;
  }

  inline int64_t cumulativePosition() {
    int64_t result;

    critical_section_enter_blocking(&_cs);
    result = _cumulativePosition;
    critical_section_exit(&_cs);

    return result;
  }

protected:
  critical_section_t _cs;
  repeating_timer_t _timer;
  uint8_t _buf[3];
  EncoderSetup _setup;

  uint32_t _position;
  int32_t _positionDifference;
  absolute_time_t _positionReadTime;
  uint32_t _lastPosition;
  absolute_time_t _lastPositionReadTime;

  int64_t _cumulativePosition;

  void inline _readPosition();
  uint32_t inline _grayToBinary(uint32_t gray);
  virtual void _loop();
};


enum EncoderDirection {
  Forwards,
  Backwards,
};


/**
 * @brief A class that simulates an encoder for testing purposes
 * 
 * Use the setSpeed method to set the speed in RPM of the 
 * simulated encoder. The direction method can be used to set
 * the direction of the simulated encoder.
 */
class EncoderSimulator : public Encoder {
  friend bool _encoderSimulatorTimerCallback(repeating_timer_t *rt);
public:
  EncoderSimulator(EncoderSetup setup) : Encoder(setup) {}

  virtual void begin() override;

  void speed(float speed);
  float speed();

  void direction(EncoderDirection direction);
  EncoderDirection direction();

protected:
  float _speed = 0.0;
  float _pulsesPerSecond = 0.0;
  EncoderDirection _direction = Forwards;

  float _stepsPerInterval;
  float _internalPosition;

  virtual void _loop() override;
};

