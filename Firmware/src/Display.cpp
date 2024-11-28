#include <Display.hpp>  
#include <algorithm>


bool _displayTimerCallback(repeating_timer_t *rt) {
  Display *display = (Display *)rt->user_data;
  display->_loop();
  return true;
}

void Display::begin() {
  critical_section_init(&_cs);

  if (_setup.displayBanner) {
    _display.setDisplayToString("HELLO");
    delay(1000);
    _display.setDisplayToString("SITE3");
    delay(1000);
    _display.clearDisplay();
    delay(1000);
  }

  _leadscrew.engage(false);

  _tpiIndex = _setup.defaultTPIThreadIndex;
  _metricIndex = _setup.defaultMetricThreadIndex;
  _tpiPitch(_setup.tpiThreads[_tpiIndex]);
  _metricPitch(_setup.metricThreads[_metricIndex]);
  powerFeedIPR(_setup.defaultIPR);
  mode(TPI);

  add_repeating_timer_ms(10, _displayTimerCallback, this, &_timer);
}

void Display::_updateIndicators() {
  uint8_t indicators = 0;

  switch(_mode) {
    case TPI: {
        uint8_t value = round(_tpiThread);

        for (int i = 3; i >= 0; i--) {
          _display.setDisplayDigit(value % 10, i, false);
          value /= 10;
        }

        indicators |= TPIMode;
      }
      break;

    case Metric: {
        uint32_t value = round(_metricThread * 100);

        for (int i = 3; i >= 0; i--) {
          _display.setDisplayDigit(value % 10, i, i == 1);
          value /= 10;
        }

        indicators |= MetricMode;
      }
      break;

    case Powerfeed: {
        uint32_t value = round(_powerFeedIPR * 1000);

        for (int i = 3; i >= 0; i--) {
          _display.setDisplayDigit(value % 10, i, i == 0);
          value /= 10;
        }

        indicators |= PowerfeedMode;
      }
      break;
  }

  uint32_t speed = _tachometer.speed();

  for (int i = 7; i >= 4; i--) {
    _display.setDisplayDigit(speed % 10, i, false);
    speed /= 10;
  }

  if (_leadscrew.engaged()) {
    indicators |= Engaged;
  }

  _display.setLEDs(indicators);
}

void Display::_readButtons() {
  absolute_time_t now = get_absolute_time();

  uint32_t buttons = _display.getButtons();

  if (!buttons) {
    _lastButtonPress = 0;
  }

  if (absolute_time_diff_us(_lastButtonPress, now) > 250000) { // 250ms repeat rate
    if (buttons & Engage) {
      _leadscrew.engage(!_leadscrew.engaged());
    }

    if (buttons & Increase) {
      _increase();
    }

    if (buttons & Decrease) {
      _decrease();
    }

    if (buttons & SetTPIMode) {
      mode(TPI);
    }

    if (buttons & SetMetricMode) {
      mode(Metric);
    }

    if (buttons & SetPowerfeedMode) {
      mode(Powerfeed);
    }

    if (buttons) {
      _lastButtonPress = now;
    }
  }
}

void Display::_loop() {
  if (absolute_time_diff_us(lastDisplayUpdateUs, get_absolute_time()) > _setup.displayUpdateIntervalMs * 1000) {
    lastDisplayUpdateUs = get_absolute_time();
    _updateIndicators();
  }

  _readButtons();
}

void Display::mode(DisplayMode mode) {
  critical_section_enter_blocking(&_cs);
  _mode = mode;
  critical_section_exit(&_cs);

  switch (mode) {
    case TPI:
      _leadscrew.threadTPI(_tpiThread);
      break;

    case Metric:
      _leadscrew.threadMetric(_metricThread);
      break;

    case Powerfeed:
      _leadscrew.powerFeedIPR(_powerFeedIPR);
      break;
  }
}

DisplayMode Display::mode() {
  DisplayMode result;

  critical_section_enter_blocking(&_cs);
  result = _mode;
  critical_section_exit(&_cs);

  return result;
}

void Display::_increase() {
  switch(_mode) {
    case TPI:
      _tpiIndex = std::clamp(_tpiIndex + 1, 0, int(_setup.tpiThreads.size() - 1));
      _tpiPitch(_setup.tpiThreads[_tpiIndex]);
      break;

    case Metric:
      _metricIndex = std::clamp(_metricIndex + 1, 0, int(_setup.metricThreads.size() - 1));
      _metricPitch(_setup.metricThreads[_metricIndex]);
      break;

    case Powerfeed:
      powerFeedIPR(_powerFeedIPR + 0.001);
      break;
  }
}

void Display::_decrease() {
  switch(_mode) {
    case TPI:
      _tpiIndex = std::clamp(_tpiIndex - 1, 0, int(_setup.tpiThreads.size() - 1));
      _tpiPitch(_setup.tpiThreads[_tpiIndex]);
      break;

    case Metric:
      _metricIndex = std::clamp(_metricIndex - 1, 0, int(_setup.metricThreads.size() - 1));
      _metricPitch(_setup.metricThreads[_metricIndex]);
      break;

    case Powerfeed:
      powerFeedIPR(_powerFeedIPR - 0.001);
      break;
  }
}

void Display::_tpiPitch(float pitch) {
  _tpiThread = pitch;
  _leadscrew.threadTPI(_tpiThread);
}

void Display::_metricPitch(float pitch) {
  _metricThread = pitch;
  _leadscrew.threadMetric(_metricThread);
}

void Display::powerFeedIPR(float feedRate) {
  _powerFeedIPR = std::clamp(feedRate, _setup.minIPR, _setup.maxIPR);
  _leadscrew.powerFeedIPR(_powerFeedIPR);
}
