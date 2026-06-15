// ═══════════════════════════════════════════════════════════════════════════════
//  SmartCane — Main Firmware
//  ESP32 Arduino IDE
//
//  Sub-phases:
//    2A: Skeleton
//    2B: Obstacle detection + vibration
//    2C: Battery monitoring
//    2D: DFPlayer Mini audio
//    2E: Button state machine
//    2F: BLE reconnection state machine
//    2G: GPS + SOS pipeline
//    2H: BLE config parameters
//    2I: Full integration
// ═══════════════════════════════════════════════════════════════════════════════

#include "config.h"
#include "state.h"
#include "dfplayer.h"
#include "ble.h"
#include "sonar.h"
#include "battery.h"
#include "button.h"

// ─── Global State ─────────────────────────────────────────────────────────────
SystemState currentState = STATE_BOOT;

// App-configured parameters (defaults from config.h, overridden by app)
int  vibrationLevel   = DEFAULT_VIBRATION_LEVEL;
int  sensitivityLevel = DEFAULT_SENSITIVITY_LEVEL;
bool buzzerEnabled    = DEFAULT_BUZZER_ENABLED;

// Battery warning flags (prevent repeated alerts per session)
bool lowBatteryWarning1Sent = false;  // 20% warning
bool lowBatteryWarning2Sent = false;  // 10% warning

// Timing
unsigned long lastBatteryCheckMs  = 0;
unsigned long lastSonarCycleMs    = 0;

// ─── Function Declarations ────────────────────────────────────────────────────
// Defined in sub-phase files; declared here so the compiler sees them all.

// Obstacle detection
void     initSonar();
int      measureDistanceCm();
void     setVibration(int distanceCm);

// Battery
void     initBattery();
int      readBatteryPercent();
void     checkAndAlertBattery();
void     reportBatteryToUser();

// Audio
void     initDFPlayer();
void     playAudio(int trackIndex);
void     waitForAudioFinish();

// Button
void     initButton();
void     handleButton();

// BLE
void     initBLE();
void     handleBLEConnection();
void     sendBLENotification(String message);

// 2G — GPS + SOS
void     initGPS();
void     triggerSOS();
bool     acquireGPSFix(float &lat, float &lon);

// 2H — Config
void     applyAppConfig(String configMessage);
void     updateSonarThresholds();

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);  // Give time to the Serial Monitor to open and establish connection
  Serial.println("=== SmartCane Booting ===");


  // Sub-phase inits will be added here one by one:
  initDFPlayer();   // 2D
  initSonar();      // 2B
  initBattery();    // 2C
  initButton();     // 2E
  // initGPS();        // 2G
  initBLE();        // 2F

  // Welcome message — blocking so user hears it fully before anything else
  playAudio(AUDIO_WELCOME);
  waitForAudioFinish();

  currentState = STATE_BLE_CONNECTING;
  Serial.println("=== Boot complete. Entering main loop. ===");
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
  switch (currentState) {

    case STATE_BOOT:
      // Should not linger here — setup() transitions out of BOOT
      break;

    case STATE_BLE_CONNECTING:
      handleBLEConnection();
      handleButton(); // Allows short press to skip
      break;

    case STATE_RUNNING:

      // Check for mid-session disconnect
      if (pendingReconnect && !bleSkipped) {
        handleMidSessionReconnect();
        break;
      }

      // Check button (short = battery, long = SOS)
      handleButton();

      // Measure distance and control vibration
      if (millis() - lastSonarCycleMs >= SONAR_CYCLE_MS) {
        lastSonarCycleMs = millis();
        int distance = measureDistanceCm();
        setVibration(distance);
        Serial.print("[SONAR] Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
      }

      // Periodic battery check
      if (millis() - lastBatteryCheckMs >= BATTERY_CHECK_INTERVAL_MS) {
        lastBatteryCheckMs = millis();
        checkAndAlertBattery();
      }

      break;

    case STATE_SOS:
      // triggerSOS();  // Phase 2G — will be called here
      // For now, bounce back to RUNNING so we're not stuck
      Serial.println("[SOS] Placeholder — returning to RUNNING.");
      currentState = STATE_RUNNING;
      break;
  }
}
