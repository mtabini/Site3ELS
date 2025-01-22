#pragma once

#include <Arduino.h>
#include <array>
#include <TM1638.h>

#include <Leadscrew.hpp>
#include <Tachometer.hpp>


class ImprovedTM1638: public TM1638 {
  public:
    ImprovedTM1638(uint8_t dataPin, uint8_t clockPin, uint8_t strobePin) : TM1638(dataPin, clockPin, strobePin) {}

    /**
     * @brief Override to introduce an additional delay, which is necessary for the TM1638
     *        to work properly.
     * 
     * @return byte The received byte
     */
    virtual byte receive() {
      byte temp = 0;

      // Pull-up on
      gpio_set_dir(dataPin, GPIO_IN);
      gpio_put(dataPin, 1);

      for (int i = 0; i < 8; i++) {
        temp >>= 1;

        gpio_put(clockPin, 0);
        busy_wait_us(10); // Additional delay

        if (gpio_get(dataPin)) {
          temp |= 0x80;
        }

        gpio_put(clockPin, 1);        
        busy_wait_us(1); // Additional delay
      }

      // Pull-up off
      gpio_set_dir(dataPin, GPIO_OUT);
      gpio_put(dataPin, 0);

      return temp;
    }
};


typedef struct {
  uint8_t clockPin;
  uint8_t dataPin;
  uint8_t strobePin;

  std::vector<float> tpiThreads;
  std::vector<float> metricThreads;

  int defaultTPIThreadIndex;
  int defaultMetricThreadIndex;

  float minIPR;
  float maxIPR;
  float defaultIPR;

  Leadscrew &leadscrew;
  Tachometer &tachometer;

  bool displayBanner;
  uint32_t displayUpdateIntervalMs;
  uint32_t displayLoopIntervalMs;
  size_t displayButtonQueueSize;

  uint32_t displayAutoOffInterval;
} DisplaySetup;


typedef enum {
  TPI,
  Metric,
  Powerfeed,
} DisplayMode;


typedef enum {
  TPIMode = 0x01,
  MetricMode = 0x02,
  PowerfeedMode = 0x04,
  ErrorA = 0x10,
  ErrorB = 0x20,
  Engaged = 0x80,
} DisplayIndicators;


typedef enum {
  SetTPIMode = 0x01,
  SetMetricMode = 0x02,
  SetPowerfeedMode = 0x04,
  Decrease = 0x10,
  Increase = 0x20,
  Engage = 0x80,
} ButtonIndices;


class Display {
  friend bool _displayTimerCallback(repeating_timer_t *rt);
  public:
    Display(DisplaySetup setup) : 
      _setup(setup),
      _display(setup.dataPin, setup.clockPin, setup.strobePin), _leadscrew(setup.leadscrew), 
      _tachometer(setup.tachometer) 
      {}

    void begin();

    void mode(DisplayMode mode);
    DisplayMode mode();

  protected:
    ImprovedTM1638 _display;
    DisplaySetup _setup;
    Leadscrew &_leadscrew;
    Tachometer &_tachometer;

    repeating_timer_t _timer;
    critical_section_t _cs;

    uint32_t lastDisplayUpdateUs = 0;

    bool _sleeping = false;
    bool _idle = false;
    uint32_t _idleStartTime = 0;

    DisplayMode _mode;

    int _tpiIndex;
    int _metricIndex;

    float _tpiThread;
    float _metricThread;
    float _powerFeedIPR;

    std::deque<uint32_t> _buttonPressQueue;

    absolute_time_t _lastButtonPress = 0;

    void _updateIndicators();
    void _readButtons();

    void _tpiPitch(float pitch);
    void _metricPitch(float pitch);
    void powerFeedIPR(float feedRate);

    void _increase();
    void _decrease();
    
    void _loop();
};
