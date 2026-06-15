#pragma once

// ═══════════════════════════════════════════════════════════════════════════════
//  SmartCane — Central Configuration
//  Edit this file to tune hardware pins, thresholds, and behavior.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── BLE
// ──────────────────────────────────────────────────────────────────────
#define BLE_DEVICE_NAME "SmartCane"
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHAR_NOTIFY_UUID "12345678-1234-1234-1234-123456789ab1" // ESP32 → App
#define CHAR_WRITE_UUID "12345678-1234-1234-1234-123456789ab2"  // App → ESP32

// ─── GPIO Pins
// ────────────────────────────────────────────────────────────────
#define PIN_TRIG 4         // HC-SR04 trigger (via logic level shifter)
#define PIN_ECHO 5         // HC-SR04 echo   (via logic level shifter)
#define PIN_BUTTON 15      // SOS / battery button (INPUT_PULLUP, to GND)
#define PIN_VIBRATION 18   // MOSFET gate — PWM vibration motor control
#define PIN_DFPLAYER_TX 25 // ESP32 TX → DFPlayer RX (via 1kΩ resistor)
#define PIN_DFPLAYER_RX 26 // ESP32 RX ← DFPlayer TX
#define PIN_BATTERY_ADC 34 // Voltage divider output (input-only ADC pin)
#define PIN_BUZZER 26      // Buzzer (direct or via 2N2222)
#define PIN_GPS_RX 16      // ESP32 RX ← Neo M6 TX
#define PIN_GPS_TX 17      // ESP32 TX → Neo M6 RX

// ─── Button Timing
// ────────────────────────────────────────────────────────────
#define BUTTON_DEBOUNCE_MS 50         // Ignore transitions shorter than this
#define BUTTON_SHORT_MAX_MS 2999      // < 3s = short press (battery report)
#define BUTTON_LONG_THRESHOLD_MS 3000 // ≥ 3s = long press (SOS or skip BLE)

// ─── HC-SR04 Obstacle Detection
// ───────────────────────────────────────────────
#define SONAR_SAMPLE_COUNT 7 // Readings per measurement cycle
#define SONAR_CYCLE_MS 80    // ms between full measurement cycles
#define SONAR_MIN_CM 5       // Below this = sensor noise, discard
#define SONAR_MAX_CM 400     // Above this = out of range, discard

// Distance thresholds — default sensitivity (MEDIUM).
// These will be overridden by app config when connected.
#define SONAR_NEAR_CM_DEFAULT 30 // Closer than this = max vibration
#define SONAR_FAR_CM_DEFAULT 120 // Farther than this = no vibration

// ─── Vibration Motor (PWM)
// ────────────────────────────────────────────────────
#define VIBRATION_PWM_FREQ 5000    // Hz
#define VIBRATION_PWM_RESOLUTION 8 // bits (0–255)
#define VIBRATION_MIN_DUTY 60      // Minimum duty to actually spin the motor
#define VIBRATION_MAX_DUTY 255     // Maximum duty

// ─── Buzzer
// ───────────────────────────────────────────────────────────────────
#define BUZZER_NEAR_CM_DEFAULT                                                 \
  30 // Only buzz when closer than this (default off via app)

// ─── Battery Monitoring
// ─────────────────────────────────────────────────────── Voltage divider:
// R1=100kΩ, R2=100kΩ → divides battery voltage by 2 ESP32 ADC reference: 3.3V,
// 12-bit resolution (0–4095)
// Battery range: 3.0V–4.2V → pin range: 1.5V–2.1V
// ESP32 ADC with ADC_11db attenuation reads 0–3.9V (safe for our 2.1V max)
#define BATTERY_ADC_RESOLUTION 4095.0f
#define BATTERY_ADC_VREF 3.3f      // Effective reference with 11db atten
#define BATTERY_DIVIDER_RATIO 2.0f // Vout = Vbat / 2
#define BATTERY_MAX_VOLTAGE 4.2f   // 18650 fully charged
#define BATTERY_MIN_VOLTAGE 3.0f   // 18650 cutoff (don't discharge below this)
#define BATTERY_CHECK_INTERVAL_MS 60000 // Check battery every 60 seconds
#define BATTERY_LOW_THRESHOLD_1 20      // First low battery warning  (%)
#define BATTERY_LOW_THRESHOLD_2 10      // Critical battery warning   (%)
#define BATTERY_ADC_OFFSET 0.172f       // Adjust if multimeter vs code differs

// ─── GPS
// ──────────────────────────────────────────────────────────────────────
#define GPS_BAUD_RATE 9600
#define GPS_FIX_TIMEOUT_MS 20000 // 20 seconds to acquire a valid fix

// ─── DFPlayer Mini
// ────────────────────────────────────────────────────────────
#define DFPLAYER_BAUD_RATE 9600
#define DFPLAYER_VOLUME 25 // 0–30

// Audio file index map — filenames on SD: 0001.mp3, 0002.mp3, ...
// Record these in this exact order and load them onto the SD card.
#define AUDIO_WELCOME 1        // "Welcome. SmartCane is starting up."
#define AUDIO_BLE_CONNECTING 2 // "Connecting to phone application..."
#define AUDIO_BLE_CONNECTED 3  // "Connected successfully."
#define AUDIO_BLE_RETRY 4      // "Could not connect. Retrying..."
#define AUDIO_BLE_SKIPPED 5    // "Running without Bluetooth connection."
#define AUDIO_BLE_LOST 6       // "Connection lost. Attempting to reconnect."
#define AUDIO_BATTERY_100 7    // "Battery: 100%"
#define AUDIO_BATTERY_90 8     // "Battery: 90%"
#define AUDIO_BATTERY_80 9     // "Battery: 80%"
#define AUDIO_BATTERY_70 10    // "Battery: 70%"
#define AUDIO_BATTERY_60 11    // "Battery: 60%"
#define AUDIO_BATTERY_50 12    // "Battery: 50%"
#define AUDIO_BATTERY_40 13    // "Battery: 40%"
#define AUDIO_BATTERY_30 14    // "Battery: 30%"
#define AUDIO_BATTERY_20 15    // "Low battery. 20% remaining."
#define AUDIO_BATTERY_10 16    // "Critical battery. 10% remaining."
#define AUDIO_SOS_TRIGGERED 17 // "SOS alert triggered. Getting location..."
#define AUDIO_GPS_FAIL 18      // "Could not get GPS location. Alert not sent."
#define AUDIO_SOS_SUCCESS 19   // "SOS alert sent successfully."
#define AUDIO_SOS_FAIL 20      // "SOS alert failed to send."
#define AUDIO_CONFIG_RECEIVED 21 // "Configuration received from app."

// ─── App Configuration Defaults
// ─────────────────────────────────────────────── Applied when running without
// BLE connection. Overridden by values received from the app.
#define DEFAULT_VIBRATION_LEVEL 2 // 1=low, 2=medium, 3=high
#define DEFAULT_SENSITIVITY_LEVEL                                              \
  2 // 1=low (detects far), 2=medium, 3=high (only close)
#define DEFAULT_BUZZER_ENABLED false

// ─── Serial Debug
// ─────────────────────────────────────────────────────────────
#define SERIAL_BAUD_RATE 115200
