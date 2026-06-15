// ═══════════════════════════════════════════════════════════════════════════════
//  button.h — Button Press Detection (Short / Long Press)
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include "state.h"

// ─── Internal state
// ───────────────────────────────────────────────────────────
static unsigned long _buttonPressedAt = 0;
static unsigned long _lastReleaseMs = 0;
static bool _buttonActive = false; // Currently being held
static bool _longFired = false;    // Long press already triggered

// ─── Forward declarations (defined in other headers)
// ──────────────────────────
void reportBatteryToUser(); // battery.h
void triggerSOS();          // gps.h  (Phase 2G)
void skipBLEConnection();   // ble.h  (Phase 2F)

// ─── Init
// ─────────────────────────────────────────────────────────────────────
void initButton() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Serial.println("[BUTTON] Initialized.");
}

// ─── Main button handler — call every loop iteration ─────────────────────────
// Behavior depends on current system state:
//
//   STATE_BLE_CONNECTING:
//     Short press → skip BLE, run without connection
//     (Long press ignored during connection phase)
//
//   STATE_RUNNING:
//     Short press (<3s, on release) → report battery percentage
//     Long press  (≥3s, while held) → trigger SOS
//
//   STATE_SOS:
//     All input ignored — SOS is in progress
//
void handleButton() {
  bool pressed = (digitalRead(PIN_BUTTON) == LOW); // PULLUP: LOW = pressed

  if (pressed) {

    if (!_buttonActive) {
      // ── Falling edge (just pressed) ──
      unsigned long now = millis();

      // Debounce: ignore if it's been less than BUTTON_DEBOUNCE_MS
      // since last release
      if (now - _lastReleaseMs < BUTTON_DEBOUNCE_MS)
        return;

      _buttonPressedAt = now;
      _buttonActive = true;
      _longFired = false;
      Serial.println("[BUTTON] Pressed.");
    }

    // ── While held: check for long press threshold ──
    if (!_longFired &&
        (millis() - _buttonPressedAt >= BUTTON_LONG_THRESHOLD_MS)) {

      _longFired = true; // Prevent re-triggering while still held

      if (currentState == STATE_RUNNING) {
        Serial.println("[BUTTON] Long press detected → SOS.");
        currentState = STATE_SOS;
        // triggerSOS() is called from the STATE_SOS case in loop()
      }
      // Long press in STATE_BLE_CONNECTING is intentionally ignored
    }

  } else {

    if (_buttonActive) {
      // ── Rising edge (just released) ──
      unsigned long heldFor = millis() - _buttonPressedAt;
      _lastReleaseMs = millis();
      _buttonActive = false;

      if (_longFired) {
        // Long press already handled on the way down — nothing to do on release
        Serial.println("[BUTTON] Released after long press.");
        return;
      }

      // Short press — only act on release, after debounce
      if (heldFor >= BUTTON_DEBOUNCE_MS) {

        if (currentState == STATE_BLE_CONNECTING) {
          Serial.println("[BUTTON] Short press during BLE connect → skip BLE.");
          // skipBLEConnection(); // Phase 2F

        } else if (currentState == STATE_RUNNING) {
          Serial.println("[BUTTON] Short press → battery report.");
          // Don't interrupt audio already playing (e.g. a low battery alert)
          if (!isAudioPlaying()) {
            reportBatteryToUser();
          }
        }
      }
    }
  }
}
