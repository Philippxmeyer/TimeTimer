#include <Adafruit_NeoPixel.h>
#include <AiEsp32RotaryEncoder.h>
#include <Arduino.h>

#include "Config.h"
#include "Feedback.h"
#include "LedDisplay.h"
#include "RotaryInput.h"
#include "TimerController.h"
#include "TimerState.h"

// ---------------------------------------------------------------------------
// Hardware instances
// ---------------------------------------------------------------------------
Adafruit_NeoPixel ring(config::kNumLeds, config::kPinNeopixel, NEO_GRB + NEO_KHZ800);
AiEsp32RotaryEncoder rotary(config::kPinRotaryDt, config::kPinRotaryClk, config::kPinRotarySw, -1);

LedDisplay display(ring);
RotaryInput rotaryInput(rotary, config::kEncoderStepsPerNotch, config::kEncoderDirection);
TimerController timer;
Feedback feedback(config::kPinPiezo, display);

// Forward ISR glue
void IRAM_ATTR readEncoderISR() { rotary.readEncoder_ISR(); }

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
void applyIdleVisuals() {
  if (timer.halfSteps() == 0) {
    display.showBoot();
  } else {
    display.showSetting(timer.halfSteps());
  }
}

void resetAll() {
  timer.reset();
  rotaryInput.setHalfSteps(0);
  applyIdleVisuals();
}

void handleButtonEvents(unsigned long now) {
  if (rotaryInput.consumeLongPress()) {
    if (timer.state() != TimerState::Idle) {
      resetAll();
    }
    return;
  }

  if (!rotaryInput.consumeShortClick()) {
    return;
  }

  switch (timer.state()) {
    case TimerState::Idle:
      if (timer.start(now)) {
        // Immediately render running state for responsive feedback.
        display.showRunning(timer.totalMinutes(), timer.elapsedMs(now), now);
      }
      break;
    case TimerState::Running:
      timer.pause(now);
      display.showRunning(timer.totalMinutes(), timer.elapsedMs(now), now);
      break;
    case TimerState::Paused:
      timer.resume(now);
      break;
    case TimerState::Finished:
      resetAll();
      break;
  }
}

void updateRunningTimer(unsigned long now) {
  if (timer.state() != TimerState::Running && timer.state() != TimerState::Paused) {
    return;
  }

  const float totalMinutes = timer.totalMinutes();
  const unsigned long elapsed = timer.elapsedMs(now);
  const unsigned long totalMs = static_cast<unsigned long>(totalMinutes * 60000.0f);

  if (timer.state() == TimerState::Running && elapsed >= totalMs) {
    timer.markFinished();
    feedback.alert(config::kAlertCycles, config::kAlertFrequencyHz, config::kAlertToneMs);
    resetAll();
    return;
  }

  display.showRunning(totalMinutes, elapsed, now);
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------
void setup() {
  display.begin(config::kDefaultBrightness);

  pinMode(config::kPinRotaryDt, INPUT_PULLUP);
  pinMode(config::kPinRotaryClk, INPUT_PULLUP);
  pinMode(config::kPinRotarySw, INPUT_PULLUP);

  rotary.begin();
  rotary.setup(readEncoderISR);
  rotaryInput.begin(config::kMaxHalfSteps);
  timer.reset();
}

void loop() {
  const unsigned long now = millis();

  rotaryInput.update(now);

  if (timer.state() == TimerState::Idle && rotaryInput.hasValueChanged()) {
    timer.setHalfSteps(rotaryInput.halfSteps());
    applyIdleVisuals();
  }

  handleButtonEvents(now);
  updateRunningTimer(now);

  delay(1);
}

