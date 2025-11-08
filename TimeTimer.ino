#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <AiEsp32RotaryEncoder.h>

// -----------------------------
// Pins (ESP32-C3 SuperMini)
// -----------------------------
#define PIN_NEOPIXEL 5
#define PIN_ROT_DT   2
#define PIN_ROT_CLK  3
#define PIN_ROT_SW   4
#define PIN_PIEZO    7   // optional, tone()

// -----------------------------
// LED-Ring & Darstellung
// -----------------------------
#define NUM_LEDS 16
uint8_t GLOBAL_BRIGHTNESS = 64;   // 0..255 (zur Laufzeit per setGlobalBrightness() änderbar)

const uint8_t WHITE_BASE = 60;    // neutrales Weiß (Hintergrund)
const uint8_t RED_FULL   = 200;   // volle Minute (rot)
const uint8_t RED_HALF   = 80;    // halbe Minute (gedimmt rot)

// -----------------------------
// Encoder-Konfiguration
// -----------------------------
// Wieviele interne Schritte erzeugt eine Rastung?
// Typisch 2 oder 4 – stelle das so ein, dass 1 Klick = 0.5 Minute ergibt.
constexpr int kEncoderStepsPerNotch = 2;

// Drehrichtung (+1 = normal, -1 = invertiert)
constexpr int kEncoderDir = +1;

// -----------------------------
Adafruit_NeoPixel ring(NUM_LEDS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
AiEsp32RotaryEncoder rotary(PIN_ROT_DT, PIN_ROT_CLK, PIN_ROT_SW, -1);

// -----------------------------
// State
// -----------------------------
enum TimerState { IDLE, RUNNING, PAUSED, FINISHED };
TimerState state = IDLE;

// 1 logischer Step = 0.5 Minute
int32_t halfSteps = 0;        // 0 .. NUM_LEDS*2
int32_t lastEncLogical = 0;   // zuletzt angezeigter logischer Wert (0.5-Min-Schritte)

unsigned long startMs        = 0;
unsigned long pausedAccumMs  = 0;
unsigned long lastFrame      = 0;
unsigned long longPressStart = 0;

// -----------------------------
// Helpers
// -----------------------------
void IRAM_ATTR readEncoderISR() { rotary.readEncoder_ISR(); }

static inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
  return ring.Color(r, g, b);
}

uint32_t mixColor(uint8_t r1,uint8_t g1,uint8_t b1,
                  uint8_t r2,uint8_t g2,uint8_t b2, float t) {
  if (t < 0) t = 0; 
  if (t > 1) t = 1;
  uint8_t r = r1 + (uint8_t)((r2 - r1) * t);
  uint8_t g = g1 + (uint8_t)((g2 - g1) * t);
  uint8_t b = b1 + (uint8_t)((b2 - b1) * t);
  return RGB(r, g, b);
}

void setGlobalBrightness(uint8_t b) {
  GLOBAL_BRIGHTNESS = b;
  ring.setBrightness(GLOBAL_BRIGHTNESS);
  ring.show();
}

unsigned long elapsedMs() {
  if (state == RUNNING) return (millis() - startMs) + pausedAccumMs;
  return pausedAccumMs;
}

float totalMinutes() {
  return halfSteps * 0.5f;
}

// -----------------------------
// LED-Ansichten
// -----------------------------
void showAllWhite() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, RGB(WHITE_BASE, WHITE_BASE, WHITE_BASE));
  }
  ring.show();
}

void showSetting() {
  // Basis: alles weiß
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, RGB(WHITE_BASE, WHITE_BASE, WHITE_BASE));
  }

  int  fullMin = halfSteps / 2;
  bool halfMin = (halfSteps % 2) == 1;

  // volle Minuten (rot)
  for (int i = 0; i < fullMin && i < NUM_LEDS; i++) {
    ring.setPixelColor(i, RGB(RED_FULL, 0, 0));
  }
  // halbe Minute (gedimmt rot)
  if (halfMin && fullMin < NUM_LEDS) {
    ring.setPixelColor(fullMin, RGB(RED_HALF, 0, 0));
  }

  ring.show();
}

void showRunning() {
  const unsigned long now = millis();
  if (now - lastFrame < 33) return; // ~30 FPS
  lastFrame = now;

  float mTot = totalMinutes();
  float mRem = mTot - (elapsedMs() / 60000.0f);
  if (mRem < 0) mRem = 0;

  int   fullRed = (int)mRem;       // volle Minuten, die noch rot sind
  float frac    = mRem - fullRed;  // 0..1 Fortschritt in aktueller Minute

  // Basis: alles weiß
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, RGB(WHITE_BASE, WHITE_BASE, WHITE_BASE));
  }

  // Volle verbleibende Minuten rot
  for (int i = 0; i < fullRed && i < NUM_LEDS; i++) {
    ring.setPixelColor(i, RGB(RED_FULL, 0, 0));
  }

  // Aktive „letzte“ LED: Rot → Weiß über die laufende Minute
  if (mRem > 0) {
    int idx = fullRed;
    if (idx >= NUM_LEDS) idx = NUM_LEDS - 1;
    float t = 1.0f - frac; // 0 = rot, 1 = weiß
    uint32_t c = mixColor(RED_FULL, 0, 0, WHITE_BASE, WHITE_BASE, WHITE_BASE, t);
    ring.setPixelColor(idx, c);
  }

  ring.show();
}

// -----------------------------
// Audio
// -----------------------------
void beep(uint16_t freq, uint16_t ms) {
  tone(PIN_PIEZO, freq, ms);
  delay(ms);
  noTone(PIN_PIEZO);
}

void blinkAndBeep() {
  for (int k = 0; k < 6; k++) {
    uint32_t col = (k % 2 == 0) ? RGB(255, 255, 255) : RGB(0, 0, 0);
    for (int i = 0; i < NUM_LEDS; i++) ring.setPixelColor(i, col);
    ring.show();
    if (k % 2 == 0) beep(2000, 120); else delay(120);
  }
}

// -----------------------------
// State-Wechsel
// -----------------------------
void startTimer() {
  if (halfSteps <= 0) return;
  pausedAccumMs = 0;
  startMs = millis();
  lastFrame = 0;
  state = RUNNING;
}

void pauseTimer() {
  pausedAccumMs = elapsedMs();
  state = PAUSED;
}

void resumeTimer() {
  startMs = millis();
  lastFrame = 0;
  state = RUNNING;
}

void resetTimer() {
  state = IDLE;
  pausedAccumMs = 0;
  halfSteps = 0;
  lastEncLogical = 0;
  rotary.setEncoderValue(0);  // Encoder (roh) nullen
  showAllWhite();             // alle weiß, bis du drehst
}

// -----------------------------
// Input (Encoder + Button)
// -----------------------------
void handleRotary() {
  if (state == IDLE && rotary.encoderChanged()) {
    long raw = rotary.readEncoder();

    // Rohbereich
    const long rawMin = 0;
    const long rawMax = (long)NUM_LEDS * 2 * kEncoderStepsPerNotch;

    // clamp
    if (raw < rawMin) { raw = rawMin; rotary.setEncoderValue(raw); }
    if (raw > rawMax) { raw = rawMax; rotary.setEncoderValue(raw); }

    // Richtung anwenden (invertiert: 0..rawMax wird gespiegelt)
    long logicalRaw = (kEncoderDir >= 0) ? raw : (rawMax - raw);

    // 1 logischer Schritt = 0.5 Minute
    long logical = logicalRaw / kEncoderStepsPerNotch; // 0..NUM_LEDS*2

    logical = constrain(logical, 0L, (long)NUM_LEDS * 2);
    if (logical != lastEncLogical) {
      lastEncLogical = logical;
      halfSteps = (int32_t)logical;

      // Rohwert zurückspiegeln (konsistent halten)
      long targetRaw = (kEncoderDir >= 0)
                       ? (halfSteps * kEncoderStepsPerNotch)
                       : (rawMax - (halfSteps * kEncoderStepsPerNotch));
      rotary.setEncoderValue(targetRaw);

      showSetting();
    }
  }

  // Kurz-Klick: Start/Pause/Weiter/Reset(FINISHED)
  if (rotary.isEncoderButtonClicked()) {
    if      (state == IDLE    && halfSteps > 0) startTimer();
    else if (state == RUNNING)                  pauseTimer();
    else if (state == PAUSED)                   resumeTimer();
    else if (state == FINISHED)                 resetTimer();
  }

  // Langdruck > 1,5 s: Reset (wenn nicht IDLE)
  if (rotary.isEncoderButtonDown()) {
    if (longPressStart == 0) longPressStart = millis();
    else if (millis() - longPressStart > 1500 && state != IDLE) {
      resetTimer();
      longPressStart = 0;
    }
  } else {
    longPressStart = 0;
  }
}

// -----------------------------
// Timer-Ablauf
// -----------------------------
void handleTimer() {
  if (state == RUNNING) {
    unsigned long totalMs = (unsigned long)(totalMinutes() * 60000.0f);
    if (elapsedMs() >= totalMs) {
      state = FINISHED;
      blinkAndBeep();
      resetTimer();
      return;
    }
    showRunning();
  }
}

// -----------------------------
// Setup / Loop
// -----------------------------
void setup() {
  // Serial.begin(115200);

  ring.begin();
  setGlobalBrightness(GLOBAL_BRIGHTNESS);
  showAllWhite();  // Boot: alles weiß

  // Pullups (robust gegen Floaten)
  pinMode(PIN_ROT_DT,  INPUT_PULLUP);
  pinMode(PIN_ROT_CLK, INPUT_PULLUP);
  pinMode(PIN_ROT_SW,  INPUT_PULLUP);

  rotary.begin();
  rotary.setup(readEncoderISR);

  // Boundaries im Rohbereich (inkl. Skalierung)
  const long rawMax = (long)NUM_LEDS * 2 * kEncoderStepsPerNotch;
  rotary.setBoundaries(0, rawMax, false);
  rotary.setAcceleration(0);                 // präzise pro Raster

  // Startwerte
  rotary.setEncoderValue(0);
  lastEncLogical = 0;
  halfSteps = 0;
}

void loop() {
  handleRotary();
  handleTimer();
  delay(1); // Scheduler-freundlich
}
