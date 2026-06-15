// ═══════════════════════════════════════════════════════════════════════════════
//  SmartCane — Main Firmware
//  ESP32 Arduino IDE
//
//  Sub-phases:
//    2A: Skeleton (current)
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
#include "dfplayer.h"
#include "sonar.h"
#include "battery.h"
#include "button.h"
#include "state.h"

// ─── Libraries ────────────────────────────────────────────────────────────────
// These will be uncommented as each sub-phase introduces them.
// Listed here so you can install them all now via Library Manager.

// #include <BLEDevice.h>          // Built-in ESP32 BLE (no install needed)
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
// #include <DFRobotDFPlayerMini.h> // Install: "DFRobotDFPlayerMini" by DFRobot
// #include <TinyGPSPlus.h>         // Install: "TinyGPSPlus" by Mikal Hart
// #include <HardwareSerial.h>      // Built-in

// ─── Global State ─────────────────────────────────────────────────────────────

// System mode — tracks what the system is currently doing
// enum SystemState {
//   STATE_BOOT,               // Initial startup sequence
//   STATE_BLE_CONNECTING,     // Trying to connect to phone
//   STATE_RUNNING,            // Normal operation (with or without BLE)
//   STATE_SOS                 // SOS triggered, acquiring GPS + sending alert
// };

SystemState currentState = STATE_BOOT;

// BLE connection flag
bool bleConnected   = false;
bool bleSkipped     = false;  // User chose to run without BLE

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

// 2B — Obstacle detection
void     initSonar();
int      measureDistanceCm();
void     setVibration(int distanceCm);

// 2C — Battery
void     initBattery();
int      readBatteryPercent();
void     checkAndAlertBattery();
void     reportBatteryToUser();

// 2D — Audio
void     initDFPlayer();
void     playAudio(int trackIndex);
void     waitForAudioFinish();

// 2E — Button
void     initButton();
void     handleButton();

// 2F — BLE
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
  // initBLE();        // 2F

  // Welcome message — blocking so user hears it fully before anything else
  playAudio(AUDIO_WELCOME);
  waitForAudioFinish();

  // ── CALIBRATION TEST (remove after calibrating) ──
  // Measure actual battery voltage with a multimeter.
  // Compare to what the code calculates.
  // Adjust BATTERY_ADC_VREF or add an offset constant if needed.
  // Serial.println("[CAL] Reading battery 100 times:");
  // for (int i = 0; i < 100; i++) {
  //   readBatteryPercent();
  //   delay(500);
  // }

  currentState = STATE_RUNNING;
  Serial.println("=== Boot complete. Entering main loop. ===");
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
  switch (currentState) {

    case STATE_BOOT:
      // Should not linger here — setup() transitions out of BOOT
      break;

    case STATE_BLE_CONNECTING:
      // 2F: Attempt BLE connection, retry loop, handle skip
      // handleBLEConnection();  // Phase 2F
      handleButton();            // ← Needed here for BLE skip (Phase 2F context)
      break;

    case STATE_RUNNING:
      handleButton();

      // 2B: Measure distance and control vibration
      if (millis() - lastSonarCycleMs >= SONAR_CYCLE_MS) {
        lastSonarCycleMs = millis();
        int distance = measureDistanceCm();
        setVibration(distance);
        Serial.print("[SONAR] Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
      }

      // 2E: Check button (short = battery, long = SOS)
      // handleButton();

      // 2C: Periodic battery check
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
