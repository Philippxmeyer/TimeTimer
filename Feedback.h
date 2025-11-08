#pragma once

#include <Arduino.h>

#include "LedDisplay.h"

class Feedback {
 public:
  Feedback(uint8_t piezoPin, LedDisplay& display);

  void beep(uint16_t frequencyHz, uint16_t durationMs);
  void alert(uint8_t cycles, uint16_t frequencyHz, uint16_t toneMs);

 private:
  uint8_t piezoPin_;
  LedDisplay& display_;
};

