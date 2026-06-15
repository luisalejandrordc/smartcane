// ═══════════════════════════════════════════════════════════════════════════════
//  gps.h — Neo M6 GPS Acquisition + SOS Pipeline
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include "state.h"
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// ─── GPS objects
// ──────────────────────────────────────────────────────────────
TinyGPSPlus gps;
HardwareSerial
    gpsSerial(2); // UART2 — Serial0=USB, Serial1=DFPlayer, Serial2=GPS

// ─── Forward declarations
// ─────────────────────────────────────────────────────
void playAudio(int track);
void waitForAudioFinish();
void stopVibration();
void sendBLENotification(String message);
extern volatile bool sosResponseReceived;
extern volatile bool sosResponseSuccess;
extern bool bleConnected;
extern bool bleSkipped;

// ─── Init
// ─────────────────────────────────────────────────────────────────────
void initGPS() {
  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  Serial.println("[GPS] Initialized.");
}

// ─── Feed GPS parser
// ────────────────────────────────────────────────────────── Call this
// regularly from loop() during normal operation so the GPS module stays warm
// and has a fix ready when SOS is triggered. A warm GPS acquires a fix in
// seconds; a cold one can take 1–2 minutes.
void feedGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

// ─── Acquire a valid GPS fix
// ────────────────────────────────────────────────── Blocks for up to
// GPS_FIX_TIMEOUT_MS (20s) waiting for valid coordinates. Returns true and
// populates lat/lon if successful, false on timeout.
bool acquireGPSFix(float &lat, float &lon) {
  Serial.println("[GPS] Waiting for fix...");
  unsigned long startMs = millis();

  while (millis() - startMs < GPS_FIX_TIMEOUT_MS) {
    // Feed all available NMEA bytes into the parser
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }

    // Valid fix: location updated, HDOP acceptable, satellite count reasonable
    if (gps.location.isValid() && gps.location.isUpdated() &&
        gps.hdop.isValid() &&
        gps.hdop.value() < 300 && // HDOP < 3.00 = good accuracy
        gps.satellites.isValid() && gps.satellites.value() >= 3) {

      lat = gps.location.lat();
      lon = gps.location.lng();

      Serial.print("[GPS] Fix acquired: ");
      Serial.print(lat, 6);
      Serial.print(", ");
      Serial.println(lon, 6);
      return true;
    }

    // Yield to prevent watchdog reset during blocking wait
    yield();
    delay(100);
  }

  Serial.println("[GPS] Fix timeout.");
  return false;
}

// ─── SOS Pipeline
// ───────────────────────────────────────────────────────────── Full flow:
//   1. Stop vibration + play SOS triggered audio
//   2. Attempt GPS fix (20s timeout)
//   3a. GPS fail → play fail audio → return to RUNNING
//   3b. GPS success → send coordinates via BLE
//   4. Wait for app response (OK/FAIL) with timeout
//   5. Play result audio → return to RUNNING
//
void triggerSOS() {
  Serial.println("[SOS] Pipeline started.");

  // Step 1 — Stop motor, alert user
  stopVibration();
  playAudio(AUDIO_SOS_TRIGGERED);
  waitForAudioFinish();

  // Step 2 — Acquire GPS fix
  float lat = 0.0f;
  float lon = 0.0f;
  bool fixAcquired = acquireGPSFix(lat, lon);

  if (!fixAcquired) {
    // Step 3a — No GPS fix
    playAudio(AUDIO_GPS_FAIL);
    waitForAudioFinish();
    currentState = STATE_RUNNING;
    return;
  }

  // Step 3b — Build and send BLE message
  // If BLE is not connected (user skipped or dropped), we can't send.
  if (!bleConnected) {
    Serial.println("[SOS] BLE not connected — cannot send alert.");
    playAudio(AUDIO_SOS_FAIL);
    waitForAudioFinish();
    currentState = STATE_RUNNING;
    return;
  }

  // Format: SOS:<lat>,<lon>  (6 decimal places ≈ 0.1m accuracy)
  String message = "SOS:";
  message += String(lat, 6);
  message += ",";
  message += String(lon, 6);

  // Reset response flags before sending
  sosResponseReceived = false;
  sosResponseSuccess = false;

  sendBLENotification(message);
  Serial.println("[SOS] Coordinates sent. Waiting for app response...");

// Step 4 — Wait for OK/FAIL from app (15s timeout)
#define SOS_RESPONSE_TIMEOUT_MS 15000
  unsigned long waitStart = millis();

  while (!sosResponseReceived &&
         millis() - waitStart < SOS_RESPONSE_TIMEOUT_MS) {
    yield();
    delay(100);
  }

  // Step 5 — Play result audio
  if (!sosResponseReceived) {
    Serial.println("[SOS] No response from app within timeout.");
    playAudio(AUDIO_SOS_FAIL);

  } else if (sosResponseSuccess) {
    Serial.println("[SOS] Alert sent successfully.");
    playAudio(AUDIO_SOS_SUCCESS);

  } else {
    Serial.println("[SOS] Alert failed to send.");
    playAudio(AUDIO_SOS_FAIL);
  }

  waitForAudioFinish();
  currentState = STATE_RUNNING;
}
