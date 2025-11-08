#include "Feedback.h"

Feedback::Feedback(uint8_t piezoPin, LedDisplay& display)
    : piezoPin_(piezoPin), display_(display) {}

void Feedback::beep(uint16_t frequencyHz, uint16_t durationMs) {
  if (piezoPin_ != 0xFF) {
    tone(piezoPin_, frequencyHz, durationMs);
    delay(durationMs);
    noTone(piezoPin_);
  } else {
    delay(durationMs);
  }
}

void Feedback::alert(uint8_t cycles, uint16_t frequencyHz, uint16_t toneMs) {
  for (uint8_t i = 0; i < cycles; ++i) {
    const bool on = (i % 2) == 0;
    if (on) {
      display_.fillColor(display_.color(255, 255, 255));
      beep(frequencyHz, toneMs);
    } else {
      display_.fillColor(display_.color(0, 0, 0));
      delay(toneMs);
    }
  }
}

