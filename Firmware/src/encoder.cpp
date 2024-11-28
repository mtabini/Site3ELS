#include <encoder.hpp>
#include "hardware/spi.h"
#include "pico/stdlib.h"

bool _encoderTimerCallback(repeating_timer_t *rt) {
  Encoder *encoder = (Encoder *)rt->user_data;
  encoder->_loop();
  return true;
}

void Encoder::begin() {
  critical_section_init_with_lock_num(&_cs, 1);

  spi_init(spi0, _setup.clockSpeed);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

  gpio_set_function(_setup.clk, GPIO_FUNC_SPI);
  gpio_set_function(_setup.miso, GPIO_FUNC_SPI);

  _readPosition(); // Ensure that we have a valid position in _lastPosition
                   // before we start the main loop

  add_repeating_timer_us(_setup.updateInterval, _encoderTimerCallback, this, &_timer);
}

void inline Encoder::_readPosition() {
  _lastPosition = _position;
  _lastPositionReadTime = _positionReadTime;

  spi_read_blocking(spi0, 0, _buf, 3);

  uint32_t data = _grayToBinary(_buf[0] << 16 | _buf[1] << 8 | _buf[2]);
  _position = data & 0xFFF; // We only case about the single-turn position
  _positionReadTime = get_absolute_time();
}

void Encoder::_loop() {
  // Read the encoder position

  _readPosition();

  // Calculate the difference between the new position and
  // the old position accounting
  // for overflow and underflow in the encoder position.

  int32_t diff = _position - _lastPosition;
  int32_t maxResolutionValue = (1 << _setup.resolutionBits);

  if (diff > maxResolutionValue / 2) {
    diff -= maxResolutionValue;
  } else if (diff < -maxResolutionValue / 2) {
    diff += maxResolutionValue;
  }

  // Update state
  critical_section_enter_blocking(&_cs);
  _positionDifference += diff;
  _cumulativePosition = _cumulativePosition + int64_t(diff);
  critical_section_exit(&_cs);
}

/**
 * @brief Convert gray code to binary
 * 
 * @param gray Gray code encoded value
 * @return uint32_t The decoded binary value
 **/
uint32_t inline Encoder::_grayToBinary(uint32_t gray) {
  uint32_t binary;
  asm volatile (
    "mov %0, %1\n"          // binary = gray
    "lsr r2, %1, #1\n"      // mask = gray >> 1
    "1:\n"
    "cmp r2, #0\n"          // while (mask != 0)
    "beq 2f\n"
    "eor %0, %0, r2\n"      // binary ^= mask
    "lsr r2, r2, #1\n"      // mask >>= 1
    "b 1b\n"
    "2:\n"
    : "=&r" (binary)        // output
    : "r" (gray)            // input
    : "r2"                  // clobbered register
  );
  return binary;
}


bool _encoderSimulatorTimerCallback(repeating_timer_t *rt) {
  EncoderSimulator *encoder = (EncoderSimulator *)rt->user_data;
  encoder->_loop();
  return true;
}

void EncoderSimulator::begin() {
  critical_section_init_with_lock_num(&_cs, 1);

  _position = 0;
  _positionDifference = 0;
  _cumulativePosition = 0;

  add_repeating_timer_us(_setup.updateInterval, _encoderTimerCallback, this, &_timer);
}

void EncoderSimulator::speed(float speed) {
  _speed = speed;

  critical_section_enter_blocking(&_cs);
  critical_section_exit(&_cs);
}

float EncoderSimulator::speed() {
  return _speed;
}

void EncoderSimulator::direction(EncoderDirection direction) {
  _direction = direction;

  critical_section_enter_blocking(&_cs);
  _stepsPerInterval = abs(_stepsPerInterval);

  if (_direction == EncoderDirection::Backwards) {
    _stepsPerInterval = -_stepsPerInterval;
  }
  critical_section_exit(&_cs);
}

EncoderDirection EncoderSimulator::direction() {
  return _direction;
}

void EncoderSimulator::_loop() {
  _internalPosition += _speed * _setup.stepsPerRevolution / 60.0 * _setup.updateInterval / 1000000.0;

  // Update state
  critical_section_enter_blocking(&_cs);
  _positionDifference += _internalPosition - _cumulativePosition;
  _cumulativePosition += _internalPosition - _cumulativePosition;
  critical_section_exit(&_cs);
}