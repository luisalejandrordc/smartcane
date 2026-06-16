// ═══════════════════════════════════════════════════════════════════════════════
//  sonar.h — HC-SR04 Distance Measurement + Vibration Motor Control
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"

// ─── Runtime thresholds (overridden by app config) ─────────────────────────────
int sonarNearCm = SONAR_NEAR_CM_DEFAULT; // ≤ this → max vibration
int sonarFarCm = SONAR_FAR_CM_DEFAULT;   // ≥ this → no vibration
int _vibMinDuty = VIBRATION_MIN_DUTY;
int _vibMaxDuty = VIBRATION_MAX_DUTY;
extern bool buzzerEnabled;

// ─── Init ──────────────────────────────────────────────────────────────────────
void initSonar() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // LEDC PWM setup for vibration motor
  ledcAttach(PIN_VIBRATION, VIBRATION_PWM_FREQ, VIBRATION_PWM_RESOLUTION);
  ledcWrite(PIN_VIBRATION, 0); // Motor off at start

  DEBUG_PRINTLN("[SONAR] Initialized.");
}

// ─── Single raw pulse measurement ─────────────────────────────────────────────
// Returns distance in cm, or -1 if out of valid range.
int rawMeasureCm() {
  // Send 10µs trigger pulse
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Measure echo pulse width (timeout = max range in µs + margin)
  // At 400cm, sound round-trip takes ~2330µs. We allow 25000µs to be safe.
  long duration = pulseIn(PIN_ECHO, HIGH, 25000);

  if (duration == 0) {
    return -1; // Timeout — no echo received
  }

  int distanceCm = duration * 0.0343f / 2.0f;

  if (distanceCm < SONAR_MIN_CM || distanceCm > SONAR_MAX_CM) {
    return -1;
  }

  return distanceCm;
}

// ─── Median of N samples (noise-filtered measurement) ─────────────────────────
// Takes SONAR_SAMPLE_COUNT readings, discards invalid ones, sorts the valid ones,
// and returns the median. Returns -1 if fewer than 3 valid readings were obtained.
int measureDistanceCm() {
  int samples[SONAR_SAMPLE_COUNT];
  int validCount = 0;

  for (int i = 0; i < SONAR_SAMPLE_COUNT; i++) {
    int d = rawMeasureCm();
    if (d != -1) {
      samples[validCount++] = d;
    }
    delayMicroseconds(1000); // 1ms between pulses — prevents echo crosstalk
  }

  if (validCount < 3) {
    // Too few valid readings — treat as no obstacle in range
    return SONAR_MAX_CM + 1;
  }

  // Insertion sort (simple and fast for small arrays)
  for (int i = 1; i < validCount; i++) {
    int key = samples[i];
    int j = i - 1;
    while (j >= 0 && samples[j] > key) {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = key;
  }

  return samples[validCount / 2]; // Median value
}

// ─── Map distance to vibration intensity ──────────────────────────────────────
// Linear interpolation between sonarFarCm (no vibration) and sonarNearCm (max vibration).
// Accounts for minimum motor spin-up duty.
void setVibration(int distanceCm) {
  if (distanceCm < 0 || distanceCm >= sonarFarCm) {
    // No obstacle in range — motor off
    ledcWrite(PIN_VIBRATION, 0);
    return;
  }

  if (distanceCm <= sonarNearCm) {
    // At or closer than near threshold — max vibration
    ledcWrite(PIN_VIBRATION, _vibMaxDuty);
    return;
  }

  // Linear interpolation between far (0 duty) and near (max duty)
  // distanceCm is between sonarNearCm and sonarFarCm
  float ratio = 1.0f - ((float)(distanceCm - sonarNearCm) /
                        (float)(sonarFarCm - sonarNearCm));

  // Map ratio to duty cycle range [VIBRATION_MIN_DUTY, VIBRATION_MAX_DUTY]
  int duty = _vibMinDuty + (int)(ratio * (_vibMaxDuty - _vibMinDuty));
  duty = constrain(duty, _vibMinDuty, _vibMaxDuty);

  ledcWrite(PIN_VIBRATION, duty);

  if (buzzerEnabled && distanceCm <= BUZZER_NEAR_CM_DEFAULT) {
    digitalWrite(PIN_BUZZER, HIGH);
  } else {
    digitalWrite(PIN_BUZZER, LOW);
  }
}

// ─── Stop vibration immediately ───────────────────────────────────────────────
// Called when entering SOS state or BLE reconnect state.
void stopVibration() { 
  ledcWrite(PIN_VIBRATION, 0); 
}
