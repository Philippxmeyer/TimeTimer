#pragma once

#include <Arduino.h>

namespace config {

// ---------------------------------------------------------------------------
// Hardware pin map (ESP32-C3 SuperMini defaults)
// ---------------------------------------------------------------------------
constexpr uint8_t kPinNeopixel = 5;
constexpr uint8_t kPinRotaryDt = 2;
constexpr uint8_t kPinRotaryClk = 3;
constexpr uint8_t kPinRotarySw = 4;
constexpr uint8_t kPinPiezo = 7;  // Optional piezo buzzer for tone()

// ---------------------------------------------------------------------------
// LED ring configuration
// ---------------------------------------------------------------------------
constexpr uint8_t kNumLeds = 16;
constexpr uint8_t kDefaultBrightness = 64;  // 0..255

constexpr uint8_t kWhiteBase = 60;  // Neutral white background
constexpr uint8_t kRedFull = 200;   // Full-minute segment
constexpr uint8_t kRedHalf = 80;    // Half-minute segment

// ---------------------------------------------------------------------------
// Rotary encoder configuration
// ---------------------------------------------------------------------------
// How many raw steps does a single mechanical detent produce?
constexpr int kEncoderStepsPerNotch = 2;
// Encoder direction: +1 keeps the default orientation, -1 mirrors it.
constexpr int kEncoderDirection = +1;

// ---------------------------------------------------------------------------
// Timing constants
// ---------------------------------------------------------------------------
constexpr unsigned long kFrameIntervalMs = 33;      // ~30 FPS display update
constexpr unsigned long kButtonDebounceMs = 35;     // Button bounce filter
constexpr unsigned long kButtonClickMinMs = 40;     // Ignore ultra-short taps
constexpr unsigned long kButtonLongPressMs = 1500;  // Hold to reset

// Alert feedback behaviour
constexpr uint8_t kAlertCycles = 6;
constexpr uint16_t kAlertFrequencyHz = 2000;
constexpr uint16_t kAlertToneMs = 120;

// Computed helpers
constexpr int32_t kMaxHalfSteps = kNumLeds * 2;  // 0.5 minute per step

}  // namespace config

