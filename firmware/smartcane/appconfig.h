// ═══════════════════════════════════════════════════════════════════════════════
//  appconfig.h — App-Driven Configuration Parameters
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include "state.h"

// ─── Forward declarations ──────────────────────────────────────────────────────
void playAudio(int track);
void waitForAudioFinish();

// Variables from sonar.h
extern int sonarNearCm;
extern int sonarFarCm;
extern int _vibMinDuty;
extern int _vibMaxDuty;

// Variables from smartcane.ino
extern bool buzzerEnabled;

// ─── Sensitivity level -> distance thresholds ──────────────────────────────────
struct SensitivityConfig {
  int farCm;
  int nearCm;
};
const SensitivityConfig SENSITIVITY_LEVELS[] = {
    {0, 0},    // index 0 unused
    {60, 10}, // Level 1 — Low (wide range)
    {50, 10}, // Level 2 — Medium (default)
    {40, 10}, // Level 3 — High (close range only)
};

// ─── Vibration level -> PWM duty cycle limits ──────────────────────────────────
struct VibrationConfig {
  int minDuty;
  int maxDuty;
};
const VibrationConfig VIBRATION_LEVELS[] = {
    {0, 0},    // index 0 unused
    {60, 200}, // Level 1 — Low
    {60, 255}, // Level 2 — Medium (default)
    {100, 255}, // Level 3 — High
};

// ─── Apply vibration level ─────────────────────────────────────────────────────
void applyVibrationLevel(int level) {
  if (level < 1 || level > 3) {
    DEBUG_PRINT("[CFG] Invalid vibration level: ");
    DEBUG_PRINTLN(level);
    return;
  }

  // Patch the PWM limits used by sonar.h at runtime
  _vibMinDuty = VIBRATION_LEVELS[level].minDuty;
  _vibMaxDuty = VIBRATION_LEVELS[level].maxDuty;

  DEBUG_PRINT("[CFG] Vibration level set to ");
  DEBUG_PRINTLN(level);
}

// ─── Apply sensitivity level ───────────────────────────────────────────────────
void applySensitivityLevel(int level) {
  if (level < 1 || level > 3) {
    DEBUG_PRINT("[CFG] Invalid sensitivity level: ");
    DEBUG_PRINTLN(level);
    return;
  }

  sonarNearCm = SENSITIVITY_LEVELS[level].nearCm;
  sonarFarCm = SENSITIVITY_LEVELS[level].farCm;

  DEBUG_PRINT("[CFG] Sensitivity level set to ");
  DEBUG_PRINT(level);
  DEBUG_PRINT(" (near=");
  DEBUG_PRINT(sonarNearCm);
  DEBUG_PRINT("cm, far=");
  DEBUG_PRINT(sonarFarCm);
  DEBUG_PRINTLN("cm)");
}

// ─── Apply buzzer toggle ───────────────────────────────────────────────────────
void applyBuzzerEnabled(bool enabled) {
  buzzerEnabled = enabled;
  if (!enabled) {
    digitalWrite(PIN_BUZZER, LOW); // Silence immediately if disabled
  }
  
  DEBUG_PRINT("[CFG] Buzzer ");
  DEBUG_PRINTLN(enabled ? "enabled." : "disabled.");
}

// ─── Parse and apply a CFG message from the app ────────────────────────────────
// Expected format: "CFG:<vibration>,<sensitivity>,<buzzer>"
// Example:         "CFG:2,3,1" (vibration=2, sensitivity=3, buzzer=on)
void applyAppConfig(String msg) {
  DEBUG_PRINT("[CFG] Received: ");
  DEBUG_PRINTLN(msg);

  // Strip "CFG:" prefix
  if (!msg.startsWith("CFG:")) {
    DEBUG_PRINTLN("[CFG] Malformed message — ignoring.");
    return;
  }
  
  String payload = msg.substring(4);

  // Parse three comma-separated integers
  int firstComma = payload.indexOf(',');
  int secondComma = payload.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) {
    DEBUG_PRINTLN("[CFG] Parse error — expected CFG:<v>,<s>,<b>");
    return;
  }

  int vibLevel  = payload.substring(0, firstComma).toInt();
  int sensLevel = payload.substring(firstComma + 1, secondComma).toInt();
  int buzzer    = payload.substring(secondComma + 1).toInt();

  applyVibrationLevel(vibLevel);
  applySensitivityLevel(sensLevel);
  applyBuzzerEnabled(buzzer == 1);

  // Confirm receipt to user via audio
  // Non-blocking — sonar and button continue while audio plays
  playAudio(AUDIO_CONFIG_RECEIVED);
  waitForAudioFinish();
}