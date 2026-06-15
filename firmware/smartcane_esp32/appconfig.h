// ═══════════════════════════════════════════════════════════════════════════════
//  appconfig.h — App-Driven Configuration Parameters
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include "state.h"

// ─── Forward declarations
// ─────────────────────────────────────────────────────
void playAudio(int track);
void waitForAudioFinish();
extern int sonarNearCm;    // sonar.h
extern int sonarFarCm;     // sonar.h
extern bool buzzerEnabled; // smartcane.ino

// ─── Sensitivity level → distance thresholds
// ────────────────────────────────── Level 1 (Low)    — detects obstacles far
// away, vibrates early Level 2 (Medium) — balanced default Level 3 (High)   —
// only vibrates when obstacle is very close
//
//                 FAR threshold    NEAR threshold
//  Level 1:          300cm            80cm
//  Level 2:          200cm            50cm   (default)
//  Level 3:          120cm            30cm

struct SensitivityConfig {
  int farCm;
  int nearCm;
};

const SensitivityConfig SENSITIVITY_LEVELS[] = {
    {0, 0},    // index 0 unused
    {150, 40}, // Level 1 — Low sensitivity (wide range)
    {120, 30}, // Level 2 — Medium (default)
    {100, 20}, // Level 3 — High sensitivity (close range only)
};

// ─── Vibration level → PWM duty cycle limits
// ────────────────────────────────── Level 1 (Low)    — subtle feedback, less
// intrusive Level 2 (Medium) — balanced default Level 3 (High)   — strong,
// unmistakable feedback
//
//               MIN duty    MAX duty
//  Level 1:       40          140
//  Level 2:       60          255   (default)
//  Level 3:       80          255   (hits max sooner in the ramp)

struct VibrationConfig {
  int minDuty;
  int maxDuty;
};

const VibrationConfig VIBRATION_LEVELS[] = {
    {0, 0},    // index 0 unused
    {60, 200}, // Level 1 — Low
    {60, 230}, // Level 2 — Medium (default)
    {80, 255}, // Level 3 — High
};

// ─── Runtime config values
// ──────────────────────────────────────────────────── These mirror the globals
// in smartcane.ino and are applied immediately when a CFG message arrives from
// the app.
int currentVibrationLevel = DEFAULT_VIBRATION_LEVEL;
int currentSensitivityLevel = DEFAULT_SENSITIVITY_LEVEL;

// ─── Apply vibration level
// ────────────────────────────────────────────────────
void applyVibrationLevel(int level) {
  if (level < 1 || level > 3) {
    Serial.print("[CFG] Invalid vibration level: ");
    Serial.println(level);
    return;
  }
  currentVibrationLevel = level;

  // Patch the PWM limits used by sonar.h at runtime
  // We expose these as externs so setVibration() picks them up immediately
  extern int _vibMinDuty;
  extern int _vibMaxDuty;
  _vibMinDuty = VIBRATION_LEVELS[level].minDuty;
  _vibMaxDuty = VIBRATION_LEVELS[level].maxDuty;

  Serial.print("[CFG] Vibration level set to ");
  Serial.println(level);
}

// ─── Apply sensitivity level
// ──────────────────────────────────────────────────
void applySensitivityLevel(int level) {
  if (level < 1 || level > 3) {
    Serial.print("[CFG] Invalid sensitivity level: ");
    Serial.println(level);
    return;
  }
  currentSensitivityLevel = level;
  sonarNearCm = SENSITIVITY_LEVELS[level].nearCm;
  sonarFarCm = SENSITIVITY_LEVELS[level].farCm;

  Serial.print("[CFG] Sensitivity level set to ");
  Serial.print(level);
  Serial.print(" (near=");
  Serial.print(sonarNearCm);
  Serial.print("cm, far=");
  Serial.print(sonarFarCm);
  Serial.println("cm)");
}

// ─── Apply buzzer toggle
// ──────────────────────────────────────────────────────
void applyBuzzerEnabled(bool enabled) {
  buzzerEnabled = enabled;
  if (!enabled)
    digitalWrite(PIN_BUZZER, LOW); // Silence immediately if disabled
  Serial.print("[CFG] Buzzer ");
  Serial.println(enabled ? "enabled." : "disabled.");
}

// ─── Parse and apply a CFG message from the app
// ─────────────────────────────── Expected format:
// "CFG:<vibration>,<sensitivity>,<buzzer>" Example:         "CFG:2,3,1"
//                   vibration=2, sensitivity=3, buzzer=on
//
void applyAppConfig(String msg) {
  Serial.print("[CFG] Received: ");
  Serial.println(msg);

  // Strip "CFG:" prefix
  if (!msg.startsWith("CFG:")) {
    Serial.println("[CFG] Malformed message — ignoring.");
    return;
  }
  String payload = msg.substring(4);

  // Parse three comma-separated integers
  int firstComma = payload.indexOf(',');
  int secondComma = payload.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) {
    Serial.println("[CFG] Parse error — expected CFG:<v>,<s>,<b>");
    return;
  }

  int vibLevel = payload.substring(0, firstComma).toInt();
  int sensLevel = payload.substring(firstComma + 1, secondComma).toInt();
  int buzzer = payload.substring(secondComma + 1).toInt();

  applyVibrationLevel(vibLevel);
  applySensitivityLevel(sensLevel);
  applyBuzzerEnabled(buzzer == 1);

  // Confirm receipt to user via audio
  playAudio(AUDIO_CONFIG_RECEIVED);
  // Non-blocking — sonar and button continue while audio plays
}
