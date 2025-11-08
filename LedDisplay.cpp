#include "LedDisplay.h"

#include <Arduino.h>

LedDisplay::LedDisplay(Adafruit_NeoPixel& ring) : ring_(ring), lastFrameMs_(0) {}

void LedDisplay::begin(uint8_t brightness) {
  ring_.begin();
  setBrightness(brightness);
  showBoot();
}

void LedDisplay::setBrightness(uint8_t brightness) {
  ring_.setBrightness(brightness);
  ring_.show();
}

void LedDisplay::showBoot() { fillColor(color(config::kWhiteBase, config::kWhiteBase, config::kWhiteBase)); }

void LedDisplay::showSetting(int32_t halfSteps) {
  for (int i = 0; i < config::kNumLeds; ++i) {
    ring_.setPixelColor(i, color(config::kWhiteBase, config::kWhiteBase, config::kWhiteBase));
  }

  const int fullMinutes = halfSteps / 2;
  const bool hasHalfMinute = (halfSteps % 2) == 1;

  for (int i = 0; i < fullMinutes && i < config::kNumLeds; ++i) {
    ring_.setPixelColor(i, color(config::kRedFull, 0, 0));
  }

  if (hasHalfMinute && fullMinutes < config::kNumLeds) {
    ring_.setPixelColor(fullMinutes, color(config::kRedHalf, 0, 0));
  }

  ring_.show();
}

void LedDisplay::showRunning(float totalMinutes, unsigned long elapsedMs, unsigned long now) {
  if (now - lastFrameMs_ < config::kFrameIntervalMs) {
    return;
  }
  lastFrameMs_ = now;

  const float remainingMinutes = max(totalMinutes - (elapsedMs / 60000.0f), 0.0f);
  const int fullRed = static_cast<int>(remainingMinutes);
  const float fraction = remainingMinutes - fullRed;

  for (int i = 0; i < config::kNumLeds; ++i) {
    ring_.setPixelColor(i, color(config::kWhiteBase, config::kWhiteBase, config::kWhiteBase));
  }

  for (int i = 0; i < fullRed && i < config::kNumLeds; ++i) {
    ring_.setPixelColor(i, color(config::kRedFull, 0, 0));
  }

  if (remainingMinutes > 0.0f) {
    int idx = fullRed;
    if (idx >= config::kNumLeds) idx = config::kNumLeds - 1;
    const float t = 1.0f - fraction;  // 0 = red, 1 = white
    ring_.setPixelColor(idx, mixColor(config::kRedFull, 0, 0,
                                      config::kWhiteBase, config::kWhiteBase, config::kWhiteBase, t));
  }

  ring_.show();
}

void LedDisplay::fillColor(uint32_t colorValue) {
  for (int i = 0; i < config::kNumLeds; ++i) {
    ring_.setPixelColor(i, colorValue);
  }
  ring_.show();
}

uint32_t LedDisplay::color(uint8_t r, uint8_t g, uint8_t b) const { return ring_.Color(r, g, b); }

uint32_t LedDisplay::mixColor(uint8_t r1, uint8_t g1, uint8_t b1,
                              uint8_t r2, uint8_t g2, uint8_t b2, float t) const {
  t = constrain(t, 0.0f, 1.0f);
  const uint8_t r = r1 + static_cast<uint8_t>((r2 - r1) * t);
  const uint8_t g = g1 + static_cast<uint8_t>((g2 - g1) * t);
  const uint8_t b = b1 + static_cast<uint8_t>((b2 - b1) * t);
  return color(r, g, b);
}

