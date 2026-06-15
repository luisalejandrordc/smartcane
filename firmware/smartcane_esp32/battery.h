// ═══════════════════════════════════════════════════════════════════════════════
//  battery.h — Battery Voltage Monitoring
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"

// ─── Internal state
// ─────────────────────────────────────────────────────────── Tracks whether
// each low-battery warning has already been played this session. Resets only on
// power cycle — intentional, we don't want repeated alerts.
static bool _lowWarning20Sent = false;
static bool _lowWarning10Sent = false;

// ─── Init
// ─────────────────────────────────────────────────────────────────────
void initBattery() {
  // GPIO 34 is input-only by hardware — no pinMode needed for ADC.
  // Configure ADC resolution to 12-bit (0–4095).
  analogReadResolution(12);

  // Attenuate input to read up to 3.3V (default on ESP32).
  // With our voltage divider, max input is ~2.1V so no attenuation needed,
  // but setting it explicitly avoids relying on default state.
  analogSetAttenuation(ADC_11db);
  Serial.println("[BATTERY] Initialized.");
}

// ─── Read average ADC value
// ─────────────────────────────────────────────────── ESP32 ADC is noisy.
// Averaging 64 samples reduces noise significantly.
int readADCAverage(int pin, int samples = 64) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return (int)(sum / samples);
}

// ─── Read battery percentage
// ────────────────────────────────────────────────── Returns percentage rounded
// to nearest 10 (10, 20, ... 100).
int readBatteryPercent() {
  int adcRaw = readADCAverage(PIN_BATTERY_ADC);

  // Convert ADC reading to voltage at the pin
  // ESP32 ADC with ADC_11db attenuation: full scale = 3.3V at 4095
  // We'll use ADC_11db in practice (see calibration note below)
  float pinVoltage = (adcRaw / BATTERY_ADC_RESOLUTION) * BATTERY_ADC_VREF;

  // Reconstruct actual battery voltage (undo the voltage divider)
  float batteryVoltage =
      (pinVoltage * BATTERY_DIVIDER_RATIO) + BATTERY_ADC_OFFSET;

  // Clamp to known battery range
  batteryVoltage =
      constrain(batteryVoltage, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);

  // Map voltage range to 0–100%
  float percent = ((batteryVoltage - BATTERY_MIN_VOLTAGE) /
                   (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) *
                  100.0f;

  // Round to nearest 10%
  int rounded = (int)(round(percent / 10.0f) * 10);
  rounded = constrain(rounded, 10, 100); // Never report below 10% or above 100%

  Serial.print("[BATTERY] ADC raw: ");
  Serial.print(adcRaw);
  Serial.print(" | Pin voltage: ");
  Serial.print(pinVoltage, 3);
  Serial.print("V | Battery: ");
  Serial.print(batteryVoltage, 3);
  Serial.print("V | Percent: ");
  Serial.print(rounded);
  Serial.println("%");

  return rounded;
}

// ─── Map percentage to audio track index
// ──────────────────────────────────────
int batteryPercentToAudioTrack(int percent) {
  switch (percent) {
  case 100:
    return AUDIO_BATTERY_100;
  case 90:
    return AUDIO_BATTERY_90;
  case 80:
    return AUDIO_BATTERY_80;
  case 70:
    return AUDIO_BATTERY_70;
  case 60:
    return AUDIO_BATTERY_60;
  case 50:
    return AUDIO_BATTERY_50;
  case 40:
    return AUDIO_BATTERY_40;
  case 30:
    return AUDIO_BATTERY_30;
  case 20:
    return AUDIO_BATTERY_20;
  case 10:
    return AUDIO_BATTERY_10;
  default:
    return AUDIO_BATTERY_10; // Fallback for anything unexpected
  }
}

// ─── On-demand battery report (triggered by short button press)
// ─────────────── Plays the audio track for the current battery level. Audio
// playback is handled by playAudio() from dfplayer.h (Phase 2D). Declared here
// as a forward reference — linked at compile time.
void playAudio(int track); // Forward declaration

void reportBatteryToUser() {
  int percent = readBatteryPercent();
  int track = batteryPercentToAudioTrack(percent);
  playAudio(track);
}

// ─── Periodic low battery alert
// ─────────────────────────────────────────────── Called every
// BATTERY_CHECK_INTERVAL_MS from the main loop. Plays warning audio only once
// per threshold crossing per session.
void checkAndAlertBattery() {
  int percent = readBatteryPercent();

  if (percent <= BATTERY_LOW_THRESHOLD_2 && !_lowWarning10Sent) {
    _lowWarning10Sent = true;
    _lowWarning20Sent = true; // Mark both so 20% doesn't fire after 10%
    // playAudio(AUDIO_BATTERY_10);
    Serial.println("[BATTERY] Critical battery warning played.");

  } else if (percent <= BATTERY_LOW_THRESHOLD_1 && !_lowWarning20Sent) {
    _lowWarning20Sent = true;
    // playAudio(AUDIO_BATTERY_20);
    Serial.println("[BATTERY] Low battery warning played.");
  }
}
