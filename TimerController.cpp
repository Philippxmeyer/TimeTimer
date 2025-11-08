#include "TimerController.h"

TimerController::TimerController() { reset(); }

void TimerController::reset() {
  state_ = TimerState::Idle;
  halfSteps_ = 0;
  startMs_ = 0;
  pausedAccumMs_ = 0;
}

void TimerController::setHalfSteps(int32_t halfSteps) { halfSteps_ = halfSteps; }

int32_t TimerController::halfSteps() const { return halfSteps_; }

float TimerController::totalMinutes() const { return halfSteps_ * 0.5f; }

bool TimerController::canStart() const { return state_ == TimerState::Idle && halfSteps_ > 0; }

bool TimerController::start(unsigned long now) {
  if (!canStart()) return false;
  state_ = TimerState::Running;
  startMs_ = now;
  pausedAccumMs_ = 0;
  return true;
}

void TimerController::pause(unsigned long now) {
  if (state_ != TimerState::Running) return;
  pausedAccumMs_ += now - startMs_;
  state_ = TimerState::Paused;
}

void TimerController::resume(unsigned long now) {
  if (state_ != TimerState::Paused) return;
  startMs_ = now;
  state_ = TimerState::Running;
}

void TimerController::markFinished() { state_ = TimerState::Finished; }

TimerState TimerController::state() const { return state_; }

unsigned long TimerController::elapsedMs(unsigned long now) const {
  if (state_ == TimerState::Running) {
    return (now - startMs_) + pausedAccumMs_;
  }
  if (state_ == TimerState::Paused) {
    return pausedAccumMs_;
  }
  if (state_ == TimerState::Finished) {
    return static_cast<unsigned long>(totalMinutes() * 60000.0f);
  }
  return pausedAccumMs_;
}

