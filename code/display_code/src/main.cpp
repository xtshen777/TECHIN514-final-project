#include <Arduino.h>

#include <Adafruit_NeoPixel.h>

// Pin configuration

// Stepper motor pins (X27 four wires)
// Use D0, D1, D2, D3 as on your schematic
const int MOTOR_PIN_A = 0;
const int MOTOR_PIN_B = 1;
const int MOTOR_PIN_C = 2;
const int MOTOR_PIN_D = 3;

// NeoPixel LED strip
const int LED_PIN      = 8;     // data pin for NeoPixel bar
const int LED_COUNT    = 8;

// Reset button
const int BUTTON_PIN   = 9;     // external 10k pullup to 3V3, active low

// Stepper configuration 

// Logical full scale for X27 pointer
const int STEPPER_MAX_STEPS = 600;
const int STEPPER_MIN_STEPS = 0;

// Simple 4 step sequence
const uint8_t STEPPER_SEQUENCE[4][4] = {
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

int currentStepIndex = 0;      // sequence index 0..3
int currentPosition  = 0;      // logical position 0..600
int targetPosition   = 0;      // desired position

unsigned long lastStepTime      = 0;
const unsigned long STEP_INTERVAL_MS = 5;   // adjust for motion speed

// NeoPixel configuration

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Current heart rate
int currentHeartRateBpm = 0;

// Button debounce 

bool lastButtonState   = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY_MS = 50;

// Heart rate mapping functions 

// Map heart rate to stepper logical position
int mapHeartRateToSteps(int bpm) {
  int minBpm = 50;
  int maxBpm = 150;

  if (bpm < minBpm) bpm = minBpm;
  if (bpm > maxBpm) bpm = maxBpm;

  float ratio = (float)(bpm - minBpm) / (float)(maxBpm - minBpm);
  int steps = STEPPER_MIN_STEPS + (int)(ratio * (STEPPER_MAX_STEPS - STEPPER_MIN_STEPS));
  return steps;
}

// Map heart rate to number of lit LEDs (0..LED_COUNT)
int mapHeartRateToLedCount(int bpm) {
  int minBpm = 50;
  int maxBpm = 150;

  if (bpm < minBpm) bpm = minBpm;
  if (bpm > maxBpm) bpm = maxBpm;

  float ratio = (float)(bpm - minBpm) / (float)(maxBpm - minBpm);
  int count = (int)round(ratio * LED_COUNT);
  if (count < 0) count = 0;
  if (count > LED_COUNT) count = LED_COUNT;
  return count;
}

// Stepper control 

void applyStepperCoils(int idx) {
  digitalWrite(MOTOR_PIN_A, STEPPER_SEQUENCE[idx][0]);
  digitalWrite(MOTOR_PIN_B, STEPPER_SEQUENCE[idx][1]);
  digitalWrite(MOTOR_PIN_C, STEPPER_SEQUENCE[idx][2]);
  digitalWrite(MOTOR_PIN_D, STEPPER_SEQUENCE[idx][3]);
}

void stepMotorOneStep(int direction) {
  if (direction > 0) {
    currentStepIndex = (currentStepIndex + 1) % 4;
    currentPosition++;
  } else if (direction < 0) {
    currentStepIndex = (currentStepIndex + 3) % 4;
    currentPosition--;
  }

  if (currentPosition < STEPPER_MIN_STEPS) currentPosition = STEPPER_MIN_STEPS;
  if (currentPosition > STEPPER_MAX_STEPS) currentPosition = STEPPER_MAX_STEPS;

  applyStepperCoils(currentStepIndex);
}

void updateStepper() {
  unsigned long now = millis();
  if (now - lastStepTime < STEP_INTERVAL_MS) {
    return;
  }

  lastStepTime = now;

  if (currentPosition < targetPosition) {
    stepMotorOneStep(+1);
  } else if (currentPosition > targetPosition) {
    stepMotorOneStep(-1);
  } else {
    // already at target
  }
}

// NeoPixel display 

// Gradient color, left light color, right deep color
uint32_t colorFromIndex(int index, int maxIndex) {
  float t = 0.0;
  if (maxIndex > 0) {
    t = (float)index / (float)maxIndex;
  }

  // light color on the left
  uint8_t r1 = 80,  g1 = 120, b1 = 255;
  // deep color on the right
  uint8_t r2 = 180, g2 = 0,   b2 = 200;

  uint8_t r = r1 + (uint8_t)((r2 - r1) * t);
  uint8_t g = g1 + (uint8_t)((g2 - g1) * t);
  uint8_t b = b1 + (uint8_t)((b2 - b1) * t);

  return strip.Color(r, g, b);
}

void updateLedBar(int bpm) {
  if (bpm <= 0) {
    // treat as reset: turn all off
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    return;
  }

  int litCount = mapHeartRateToLedCount(bpm);

  for (int i = 0; i < LED_COUNT; i++) {
    if (i < litCount) {
      uint32_t c = colorFromIndex(i, LED_COUNT - 1);
      strip.setPixelColor(i, c);
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

// Reset behavior 

void resetDisplayState() {
  currentHeartRateBpm = 0;
  targetPosition      = STEPPER_MIN_STEPS;

  // move pointer gradually back to minimum
  // we let updateStepper handle it in loop
  updateLedBar(0);  // clear LED bar
}

// Heart rate update entry 

// This function is called whenever new heart rate data is received.
// Later you can call this from your BLE callback.
void onHeartRateReceived(int bpm) {
  currentHeartRateBpm = bpm;
  targetPosition      = mapHeartRateToSteps(bpm);
  updateLedBar(bpm);
}

// Setup and loop 

void setup() {
  Serial.begin(115200);

  // Stepper pins
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
  pinMode(MOTOR_PIN_C, OUTPUT);
  pinMode(MOTOR_PIN_D, OUTPUT);
  applyStepperCoils(0);

  // Button
  pinMode(BUTTON_PIN, INPUT);   // external pullup to 3V3

  // NeoPixel
  strip.begin();
  strip.setBrightness(80);      // adjust brightness if needed
  strip.show();                 // clear all LEDs

  // Placeholder: Initialize BLE and set up heart rate callback

  resetDisplayState();
}

void loop() {
  // 1. Stepper follows target position
  updateStepper();

  // 2. Reset button check (active low)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = reading;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (reading == LOW) {
      // button pressed
      resetDisplayState();
      // small delay to avoid repeated triggers
      delay(300);
    }
  }

  // 3. Temporary simulation for testing without BLE
  // Uncomment this block to see pointer and LED respond.
  /*
  static unsigned long lastSimTime = 0;
  static int fakeBpm = 60;
  if (millis() - lastSimTime > 1000) {
    lastSimTime = millis();
    fakeBpm += 5;
    if (fakeBpm > 140) fakeBpm = 60;
    onHeartRateReceived(fakeBpm);
    Serial.print("Simulated BPM: ");
    Serial.println(fakeBpm);
  }
  */
}