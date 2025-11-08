#include "RotaryInput.h"

RotaryInput::RotaryInput(AiEsp32RotaryEncoder& encoder, int stepsPerNotch, int direction)
    : encoder_(encoder),
      stepsPerNotch_(stepsPerNotch),
      direction_(direction >= 0 ? 1 : -1),
      maxHalfSteps_(config::kMaxHalfSteps),
      halfSteps_(0),
      lastReported_(0),
      valueChanged_(false),
      shortClick_(false),
      longPress_(false),
      rawButtonState_(false),
      debouncedButtonState_(false),
      lastDebounceMs_(0),
      pressStartMs_(0),
      longPressReported_(false) {}

void RotaryInput::begin(int32_t maxHalfSteps) {
  maxHalfSteps_ = maxHalfSteps;
  const long rawMax = static_cast<long>(maxHalfSteps_) * stepsPerNotch_;
  encoder_.setBoundaries(0, rawMax, false);
  encoder_.setAcceleration(0);
  encoder_.setEncoderValue(0);
  halfSteps_ = 0;
  lastReported_ = 0;
}

void RotaryInput::update(unsigned long now) {
  valueChanged_ = false;
  shortClick_ = false;
  longPress_ = false;

  updateEncoder();
  updateButton(now);
}

bool RotaryInput::hasValueChanged() const { return valueChanged_; }

int32_t RotaryInput::halfSteps() const { return halfSteps_; }

void RotaryInput::setHalfSteps(int32_t value) {
  value = constrain(value, 0, maxHalfSteps_);
  halfSteps_ = value;
  lastReported_ = value;
  valueChanged_ = false;
  applyRawValue(value);
}

bool RotaryInput::consumeShortClick() {
  const bool wasClicked = shortClick_;
  shortClick_ = false;
  return wasClicked;
}

bool RotaryInput::consumeLongPress() {
  const bool wasLong = longPress_;
  longPress_ = false;
  return wasLong;
}

void RotaryInput::updateEncoder() {
  if (!encoder_.encoderChanged()) {
    return;
  }

  long raw = encoder_.readEncoder();
  const long rawMin = 0;
  const long rawMax = static_cast<long>(maxHalfSteps_) * stepsPerNotch_;

  if (raw < rawMin) {
    raw = rawMin;
    encoder_.setEncoderValue(raw);
  }
  if (raw > rawMax) {
    raw = rawMax;
    encoder_.setEncoderValue(raw);
  }

  long logicalRaw = (direction_ >= 0) ? raw : (rawMax - raw);
  long logical = logicalRaw / stepsPerNotch_;
  logical = constrain(logical, 0L, static_cast<long>(maxHalfSteps_));

  if (logical != lastReported_) {
    lastReported_ = logical;
    halfSteps_ = static_cast<int32_t>(logical);
    applyRawValue(halfSteps_);
    valueChanged_ = true;
  }
}

void RotaryInput::updateButton(unsigned long now) {
  const bool rawState = encoder_.isEncoderButtonDown();

  if (rawState != rawButtonState_) {
    rawButtonState_ = rawState;
    lastDebounceMs_ = now;
  }

  if ((now - lastDebounceMs_) >= config::kButtonDebounceMs && rawState != debouncedButtonState_) {
    debouncedButtonState_ = rawState;
    if (debouncedButtonState_) {
      pressStartMs_ = now;
      longPressReported_ = false;
    } else {
      if (!longPressReported_) {
        const unsigned long pressDuration = now - pressStartMs_;
        if (pressDuration >= config::kButtonLongPressMs) {
          longPress_ = true;
        } else if (pressDuration >= config::kButtonClickMinMs) {
          shortClick_ = true;
        }
      }
      pressStartMs_ = 0;
      longPressReported_ = false;
    }
  }

  if (debouncedButtonState_ && !longPressReported_ && pressStartMs_ != 0 &&
      (now - pressStartMs_) >= config::kButtonLongPressMs) {
    longPress_ = true;
    longPressReported_ = true;
  }
}

void RotaryInput::applyRawValue(long logicalHalfSteps) {
  const long rawMax = static_cast<long>(maxHalfSteps_) * stepsPerNotch_;
  const long rawValue = static_cast<long>(logicalHalfSteps) * stepsPerNotch_;
  const long targetRaw = (direction_ >= 0) ? rawValue : (rawMax - rawValue);
  encoder_.setEncoderValue(targetRaw);
}

