// ═══════════════════════════════════════════════════════════════════════════════
//  smartcane.ino — Main Firmware Entry Point
//  SmartCane v1.0
// ═══════════════════════════════════════════════════════════════════════════════

#include "config.h"
#include "state.h"
#include "dfplayer.h"
#include "ble.h"
#include "sonar.h"
#include "battery.h"
#include "gps.h"
#include "appconfig.h"
#include "button.h"

// ─── Global state ─────────────────────────────────────────────────────────────
SystemState currentState = STATE_BOOT;
bool buzzerEnabled = DEFAULT_BUZZER_ENABLED;

// ─── Timing ───────────────────────────────────────────────────────────────────
unsigned long lastBatteryCheckMs = 0;
unsigned long lastSonarCycleMs   = 0;

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  DEBUG_PRINTLN("=== SmartCane v1.0 Booting ===");

  // Hardware init
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  initDFPlayer();
  initSonar();
  initBattery();
  initButton();
  initGPS();
  initBLE();

  // Apply default configuration
  applyVibrationLevel(DEFAULT_VIBRATION_LEVEL);
  applySensitivityLevel(DEFAULT_SENSITIVITY_LEVEL);
  applyBuzzerEnabled(DEFAULT_BUZZER_ENABLED);

  // Welcome
  playAudio(AUDIO_WELCOME);
  waitForAudioFinish();

  currentState = STATE_BLE_CONNECTING;
  DEBUG_PRINTLN("=== Boot complete. ===");
}

// ─── Main loop ────────────────────────────────────────────────────────────────
void loop() {
  switch (currentState) {

    // ── Should never linger here ──────────────────────────────────────────────
    case STATE_BOOT:
      break;

    // ── BLE connection phase ──────────────────────────────────────────────────
    case STATE_BLE_CONNECTING:
      handleBLEConnection();
      handleButton();
      break;

    // ── Normal operation ──────────────────────────────────────────────────────
    case STATE_RUNNING:

      // Mid-session BLE disconnect — pause and reconnect
      if (pendingReconnect && !bleSkipped) {
        handleMidSessionReconnect();
        break;
      }

      handleButton();
      feedGPS();

      // Obstacle detection cycle
      if (millis() - lastSonarCycleMs >= SONAR_CYCLE_MS) {
        lastSonarCycleMs = millis();
        int distance = measureDistanceCm();
        setVibration(distance);
        DEBUG_PRINT("[SONAR] Distance: ");
        DEBUG_PRINT(distance);
        DEBUG_PRINTLN("cm");
      }

      // Periodic battery check
      if (millis() - lastBatteryCheckMs >= BATTERY_CHECK_INTERVAL_MS) {
        lastBatteryCheckMs = millis();
        checkAndAlertBattery();
      }
      break;

    // ── SOS pipeline ──────────────────────────────────────────────────────────
    case STATE_SOS:
      triggerSOS();
      break;
  }
}