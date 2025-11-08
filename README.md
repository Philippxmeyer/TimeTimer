# TimeTimer

An ESP32-based re-imagining of the classic children's *Time Timer* that uses a
WS2812B/NeoPixel LED ring for visual feedback and a rotary encoder for input.
The sketch has been modularised so you can adapt it to different hardware or
extend the behaviour without touching the core loop logic.

## Features

- Smooth LED animations that fade the active minute as time elapses.
- Rotary encoder driven UI with robust debouncing for both rotation and the
  integrated push button.
- Pause/resume support and a long-press reset gesture.
- Optional piezo buzzer feedback when the timer completes.

## Hardware

The defaults target the ESP32-C3 SuperMini but you can change the pin map in
[`Config.h`](Config.h).

| Peripheral          | Pin | Notes                                    |
| ------------------- | --- | ---------------------------------------- |
| NeoPixel ring (DIN) |  5  | Powered from 5 V with common ground.     |
| Rotary DT           |  2  | Internal pull-ups are enabled in setup(). |
| Rotary CLK          |  3  |                                          |
| Rotary SW           |  4  |                                          |
| Piezo buzzer        |  7  | Set to `0xFF` in `Config.h` to disable.  |

The LED ring is assumed to contain 16 pixels; each logical step represents half
a minute, so the full scale is 0–8 minutes. Adjust `kNumLeds` and
`kEncoderStepsPerNotch` if you use different hardware.

## Building & Uploading

1. Install the required Arduino libraries:
   - [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
   - [AiEsp32RotaryEncoder](https://github.com/igorantolic/AiEsp32RotaryEncoder)
2. Open `TimeTimer.ino` in the Arduino IDE or compile using `arduino-cli`.
3. Select the appropriate ESP32 board definition and your serial port.
4. Upload the sketch and open the serial monitor if you want to add debugging
   output (disabled by default).

## Operation

- Rotate the encoder while the timer is idle to choose the duration in
  half-minute steps. The LED ring shows full minutes in bright red and a half
  minute in dim red.
- Click the encoder button to start. A short click pauses/resumes, while a
  long (>1.5 s) press resets the timer.
- When the countdown reaches zero the ring flashes and the piezo beeps (if
  connected). Another click readies the timer for the next run.

## Extending the Project

The sketch is split into small, focused classes:

- `LedDisplay` manages the ring animations and brightness.
- `RotaryInput` debounces the encoder and button while exposing simple events.
- `TimerController` contains the state machine for the countdown.
- `Feedback` drives the optional buzzer and provides a simple alert routine.

Tweak the constants in [`Config.h`](Config.h) to adapt the timer to different
mechanical encoders, LED counts or feedback preferences.

