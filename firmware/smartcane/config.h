// ═══════════════════════════════════════════════════════════════════════════════
//  config.h — SmartCane Central Configuration
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once

// ─── BLE ──────────────────────────────────────────────────────────────────────
#define BLE_DEVICE_NAME         "SmartCane"
#define SERVICE_UUID            "12345678-1234-1234-1234-123456789abc"
#define CHAR_NOTIFY_UUID        "12345678-1234-1234-1234-123456789ab1" // ESP32 → App
#define CHAR_WRITE_UUID         "12345678-1234-1234-1234-123456789ab2"  // App → ESP32
#define BLE_CONNECT_TIMEOUT_MS  10000

// ─── GPIO Pins ────────────────────────────────────────────────────────────────
#define PIN_TRIG                4
#define PIN_ECHO                5
#define PIN_BUTTON              15
#define PIN_VIBRATION           18
#define PIN_DFPLAYER_TX         25
#define PIN_DFPLAYER_RX         26
#define PIN_BATTERY_ADC         34
#define PIN_BUZZER              27
#define PIN_GPS_TX              17
#define PIN_GPS_RX              16

// ─── Button ───────────────────────────────────────────────────────────────────
#define BUTTON_DEBOUNCE_MS        50
#define BUTTON_LONG_THRESHOLD_MS  3000

// ─── HC-SR04 ──────────────────────────────────────────────────────────────────
#define SONAR_SAMPLE_COUNT        7
#define SONAR_CYCLE_MS            80
#define SONAR_MIN_CM              5
#define SONAR_MAX_CM              400
#define SONAR_NEAR_CM_DEFAULT     30
#define SONAR_FAR_CM_DEFAULT      120

// ─── Vibration Motor ──────────────────────────────────────────────────────────
#define VIBRATION_PWM_FREQ        5000
#define VIBRATION_PWM_RESOLUTION  8
#define VIBRATION_MIN_DUTY        60
#define VIBRATION_MAX_DUTY        255

// ─── Buzzer ───────────────────────────────────────────────────────────────────
#define BUZZER_NEAR_CM_DEFAULT    30

// ─── Battery ──────────────────────────────────────────────────────────────────
#define BATTERY_ADC_RESOLUTION    4095.0f
#define BATTERY_ADC_VREF          3.3f
#define BATTERY_DIVIDER_RATIO     2.0f
#define BATTERY_MAX_VOLTAGE       4.2f
#define BATTERY_MIN_VOLTAGE       3.0f
#define BATTERY_ADC_OFFSET        0.172f
#define BATTERY_CHECK_INTERVAL_MS 60000
#define BATTERY_LOW_THRESHOLD_1   20
#define BATTERY_LOW_THRESHOLD_2   10

// ─── GPS ──────────────────────────────────────────────────────────────────────
#define GPS_BAUD_RATE             9600
#define GPS_FIX_TIMEOUT_MS        40000

// ─── DFPlayer Mini ────────────────────────────────────────────────────────────
#define DFPLAYER_BAUD_RATE        9600
#define DFPLAYER_VOLUME           25

// ─── Audio Track Index Map ────────────────────────────────────────────────────
#define AUDIO_WELCOME             1
#define AUDIO_BLE_CONNECTING      2
#define AUDIO_BLE_CONNECTED       3
#define AUDIO_BLE_RETRY           4
#define AUDIO_BLE_SKIPPED         5
#define AUDIO_BLE_LOST            6
#define AUDIO_BATTERY_100         7
#define AUDIO_BATTERY_90          8
#define AUDIO_BATTERY_80          9
#define AUDIO_BATTERY_70          10
#define AUDIO_BATTERY_60          11
#define AUDIO_BATTERY_50          12
#define AUDIO_BATTERY_40          13
#define AUDIO_BATTERY_30          14
#define AUDIO_BATTERY_20          15
#define AUDIO_BATTERY_10          16
#define AUDIO_SOS_TRIGGERED       17
#define AUDIO_GPS_FAIL            18
#define AUDIO_SOS_SUCCESS         19
#define AUDIO_SOS_FAIL            20
#define AUDIO_CONFIG_RECEIVED     21

// ─── App Config Defaults ──────────────────────────────────────────────────────
#define DEFAULT_VIBRATION_LEVEL   2
#define DEFAULT_SENSITIVITY_LEVEL 2
#define DEFAULT_BUZZER_ENABLED    false

// ─── Serial ───────────────────────────────────────────────────────────────────
#define SERIAL_BAUD_RATE          115200

// ─── Debug mode ───────────────────────────────────────────────────────────────
// Set to 1 during development, 0 for final deployment.
// When 0, all Serial.print() calls compile to nothing — saves RAM and flash.
#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif
