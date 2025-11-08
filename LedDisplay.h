#pragma once

#include <Adafruit_NeoPixel.h>

#include "Config.h"

class LedDisplay {
 public:
  explicit LedDisplay(Adafruit_NeoPixel& ring);

  void begin(uint8_t brightness = config::kDefaultBrightness);
  void setBrightness(uint8_t brightness);

  void showBoot();
  void showSetting(int32_t halfSteps);
  void showRunning(float totalMinutes, unsigned long elapsedMs, unsigned long now);
  void fillColor(uint32_t color);

  uint32_t color(uint8_t r, uint8_t g, uint8_t b) const;

 private:
  uint32_t mixColor(uint8_t r1, uint8_t g1, uint8_t b1,
                    uint8_t r2, uint8_t g2, uint8_t b2, float t) const;

  Adafruit_NeoPixel& ring_;
  unsigned long lastFrameMs_;
};

