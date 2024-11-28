#include <SerialDebug.hpp>


bool _serialDebugTimerCallback(repeating_timer_t *rt) {
  SerialDebug *serialDebug = (SerialDebug *)rt->user_data;
  serialDebug->_loop();
  return true;
}

void SerialDebug::begin() {
  add_repeating_timer_ms(_setup.updateInterval, _serialDebugTimerCallback, this, &_timer);

  Serial.begin(115200);
  Serial.println("Site 3 Electronic Leadscrew Driver");
}

void SerialDebug::_loop() {
  Serial.printf("Encoder position: %d | Speed: %f",
    _setup.encoder.cumulativePosition(),
    _setup.tachometer.speed()
  );

  switch (_setup.display.mode()) {
    case TPI:
      Serial.printf(" | Mode: TPI | Pitch: %.1f", _setup.leadscrew.threadTPI());
      break;

    case Metric:
      Serial.printf(" | Mode: Metric | Pitch: %.1f (%.2f TPI)", _setup.leadscrew.threadMetric(), _setup.leadscrew.threadTPI());
      break;

    case Powerfeed:
      Serial.printf(" | Mode: Powerfeed | IPR: %.3f", _setup.leadscrew.powerFeedIPR());
      break;
  }

#ifdef SIMULATE_ENCODER
  if (_setup.encoder.direction() == Forwards) {
    Serial.print(" | Direction: FWD");
  } else {
    Serial.print(" | Direction: CCW");
  } 
#endif // SIMULATE_ENCODER

  Serial.print("\r");

  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case 't':
        _setup.display.mode(TPI);
        break;

      case 'm':
        _setup.display.mode(Metric);
        break;

      case 'p':
        _setup.display.mode(Powerfeed);
        break;

      case 'e':
        _setup.leadscrew.engage(!_setup.leadscrew.engaged());
        break;

#ifdef SIMULATE_ENCODER

      case '.':
      case '>':
        _setup.encoder.speed(_setup.encoder.speed() + 10);
        break;

      case ',':
      case '<':
        _setup.encoder.speed(_setup.encoder.speed() - 10);
        break;
        
#endif // SIMULATE_ENCODER 
    }
  }
}

