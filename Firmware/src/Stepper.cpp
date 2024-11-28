#include <stepper.hpp>


void Stepper::begin() {
  gpio_init(_setup.directionPin);
  gpio_init(_setup.pulsePin);

  gpio_set_dir(_setup.directionPin, GPIO_OUT);
  gpio_set_dir(_setup.pulsePin, GPIO_OUT);

  gpio_put(_setup.directionPin, LOW);
}

void Stepper::loop() {
  // Do nothing if not enabled

  if (!_enabled) {
    return;
  }

  if (_stepping) {
    gpio_put(_setup.pulsePin, LOW);
    _stepping = false;
    return;
  }

  // Move towards desired position setting the direction pin in accordance
  // with the delta between the current and desired position

  int32_t delta = position - desiredPosition;

  if (delta == 0) {
    return;
  }

  if (delta < 0) {
    gpio_put(_setup.directionPin, HIGH);
    position++;
  } else if (delta > 0) {
    gpio_put(_setup.directionPin, LOW);
    position--;
  }

  // Step the motor

  gpio_put(_setup.pulsePin, HIGH);
  _stepping = true;
}

void Stepper::enabled(bool enabled) {
  if (!_enabled && enabled) {
    position = desiredPosition;
  }

  _enabled = enabled;
}

bool Stepper::enabled() {
  return _enabled;
}
