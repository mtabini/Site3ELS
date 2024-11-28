#include <Tachometer.hpp>

bool _tachometerTimerCallback(repeating_timer_t *rt) {
  Tachometer *tachometer = (Tachometer *)rt->user_data;
  tachometer->_loop();
  return true;
}

void Tachometer::begin() {
  critical_section_init_with_lock_num(&_cs, 2);

  _lastPosition = _encoder.cumulativePosition();
  _lastPositionReadTime = get_absolute_time();

  add_repeating_timer_us(10000, _tachometerTimerCallback, this, &_timer);
}

void Tachometer::_loop() {
  int64_t position = _encoder.cumulativePosition();
  absolute_time_t positionReadTime = get_absolute_time();

  int64_t positionDifference = position - _lastPosition;
  absolute_time_t timeDifference = absolute_time_diff_us(_lastPositionReadTime, positionReadTime);

  float currentSpeed = (float)positionDifference / (float)timeDifference / _encoder.stepsPerRevolution() * 60.0 * 1000000.0;

  // Save the current speed in the history buffer

  memcpy(&_speedHistory[1], _speedHistory, sizeof(float) * 9);

  _speedHistory[0] = currentSpeed;

  // Calculate the average speed

  float sum = 0.0;

  for (int i = 0; i < 10; i++) {
    sum += _speedHistory[i];
  }

  float speed = fabs(sum) / 10.0;

  critical_section_enter_blocking(&_cs);
  _speed = speed;
  critical_section_exit(&_cs);

  _lastPosition = position;
  _lastPositionReadTime = positionReadTime;
}

