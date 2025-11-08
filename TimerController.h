#pragma once

#include <Arduino.h>

#include "TimerState.h"

class TimerController {
 public:
  TimerController();

  void reset();

  void setHalfSteps(int32_t halfSteps);
  int32_t halfSteps() const;
  float totalMinutes() const;

  bool canStart() const;
  bool start(unsigned long now);
  void pause(unsigned long now);
  void resume(unsigned long now);
  void markFinished();

  TimerState state() const;
  unsigned long elapsedMs(unsigned long now) const;

 private:
  TimerState state_;
  int32_t halfSteps_;
  unsigned long startMs_;
  unsigned long pausedAccumMs_;
};

