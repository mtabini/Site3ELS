#pragma once

#include <Arduino.h>


// #define SIMULATE_ENCODER // Comment out to use the actual encoder


struct {
  // Encoder setup

  pin_size_t EncoderMOSIPin                   =      16;  // SPI MOSI
  pin_size_t EncoderClkPin                    =      18;  // SPI CLK - Note: these pins must coincide with the SPI peripheral used.
                                                          // For SPI0, the pins are 16 and 18. For SPI1, the pins are 10 and 11.
  uint32_t   EncoderClockSpeed                = 6000000;  // 6MHz
  uint8_t    EncoderResolutionBits            =      12;  // 12-bit resolution
  float      EncoderStepsPerRevolution        =    1024;  // 1024 steps per revolution
  uint32_t   EncoderUpdateInterval            =      10;  // poll interval in microseconds

  // Stepper setup

  pin_size_t StepperDirectionPin              =      21;
  pin_size_t StepperPulsePin                  =      20;
  float      StepperStepsPerRevolution        =    2400;   // 2,400 steps per revolution

  // Display setup

  pin_size_t DisplayStbPin                    =      26;   // STB pin
  pin_size_t DisplayClkPin                    =      27;   // CLK pin
  pin_size_t DisplayDioPin                    =      28;   // DIO pin

  std::vector<float> DisplayTPIThreads        =      { 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 32, 36, 40, 44, 48, 56, 64, 72, 80 };
  std::vector<float> DisplayMetricThreads     =      { 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.6, 0.7, 0.75, 0.8, 1, 1.25, 1.5, 1.75, 2, 2.5, 3 };

  int        DisplayDefaultTPIThreadIndex     =       7;   // Default TPI thread index
  int        DisplayDefaultMetricThreadIndex  =       9;   // Default metric thread index

  float      DisplayMinPowerFeedIPR           =   0.001;   // Minimum power feed setting allowed (in inches per revolution)
  float      DisplayMaxPowerFeedIPR           =   0.100;   // Maximum power feed setting allowed (in inches per revolution)

  float      DisplayDefaultPowerFeedIPR       =   0.005;   // Default power feed setting (in inches per revolution)

  bool       DisplayBanner                    =    true;   // Display banner on startup
  uint32_t   DisplayUpdateInterval            =     100;   // Update interval in milliseconds
  uint32_t   DisplayLoopInterval              =      50;   // Loop interval in milliseconds
  uint32_t   DisplayAutoOffInterval           =     600;   // Auto-off interval in seconds
  size_t     DisplayButtonQueueSize           =       3;   // Number of times the button input from the display board must be the same to be considered a valid press
  
  // Lead screw setup

  uint8_t    LeadScrewPitch                   =       8;   // 8 TPI
  uint16_t    LeadScrewReductionFactor        =     275;   // 2.75:1 reduction factor from the stepper motor to the lead screw

  // Serial debug setup

  uint32_t   SerialDebugUpdateInterval        =     100;   // Update interval in milliseconds

} Config;