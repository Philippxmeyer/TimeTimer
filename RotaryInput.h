#pragma once

#include <AiEsp32RotaryEncoder.h>
#include <Arduino.h>

#include "Config.h"

class RotaryInput {
 public:
  RotaryInput(AiEsp32RotaryEncoder& encoder, int stepsPerNotch, int direction);

  void begin(int32_t maxHalfSteps);
  void update(unsigned long now);

  bool hasValueChanged() const;
  int32_t halfSteps() const;
  void setHalfSteps(int32_t value);

  bool consumeShortClick();
  bool consumeLongPress();

 private:
  void updateEncoder();
  void updateButton(unsigned long now);
  void applyRawValue(long logicalHalfSteps);

  AiEsp32RotaryEncoder& encoder_;
  int stepsPerNotch_;
  int direction_;
  int32_t maxHalfSteps_;

  int32_t halfSteps_;
  int32_t lastReported_;
  bool valueChanged_;

  bool shortClick_;
  bool longPress_;

  bool rawButtonState_;
  bool debouncedButtonState_;
  unsigned long lastDebounceMs_;
  unsigned long pressStartMs_;
  bool longPressReported_;
};

