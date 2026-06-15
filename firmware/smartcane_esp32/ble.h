// ═══════════════════════════════════════════════════════════════════════════════
//  ble.h — BLE Server, Connection Management & Config Reception
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include "state.h"
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// ─── BLE objects
// ──────────────────────────────────────────────────────────────
BLEServer *pServer = nullptr;
BLECharacteristic *pNotifyChar = nullptr;
BLECharacteristic *pWriteChar = nullptr;

// ─── Connection state
// ─────────────────────────────────────────────────────────
bool bleConnected = false;
bool bleSkipped = false;       // User chose to run without BLE
bool bleWasConnected = false;  // Tracks previous state to detect drops
bool pendingReconnect = false; // True when connection was lost mid-session

// ─── SOS response flag
// ──────────────────────────────────────────────────────── Set by
// WriteCallbacks when app responds OK or FAIL after SOS
volatile bool sosResponseReceived = false;
volatile bool sosResponseSuccess = false;

// ─── Forward declarations
// ─────────────────────────────────────────────────────
void playAudio(int track);
void stopAudio();
void waitForAudioFinish();
void stopVibration();
void applyAppConfig(String msg); // Phase 2H

// ─── BLE Server Callbacks
// ─────────────────────────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    bleConnected = true;
    bleWasConnected = true;
    Serial.println("[BLE] Phone connected.");
  }

  void onDisconnect(BLEServer *pServer) override {
    bleConnected = false;
    Serial.println("[BLE] Phone disconnected.");

    // If we were in normal operation, flag for reconnect
    if (currentState == STATE_RUNNING) {
      pendingReconnect = true;
    }

    // Always restart advertising so phone can reconnect
    pServer->startAdvertising();
  }
};

// ─── BLE Write Callbacks
// ────────────────────────────────────────────────────── Handles all messages
// written by the app to the ESP32. Two categories: SOS responses (OK/FAIL) and
// config messages (CFG:...)
class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    String value = pChar->getValue().c_str();
    value.trim();
    Serial.print("[BLE] Received from app: ");
    Serial.println(value);

    if (value == "OK") {
      sosResponseReceived = true;
      sosResponseSuccess = true;

    } else if (value == "FAIL") {
      sosResponseReceived = true;
      sosResponseSuccess = false;

    } else if (value.startsWith("CFG:")) {
      applyAppConfig(value);

    } else {
      Serial.println("[BLE] Unknown message — ignored.");
    }
  }
};

// ─── Init
// ─────────────────────────────────────────────────────────────────────
void initBLE() {
  BLEDevice::init(BLE_DEVICE_NAME);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Notify characteristic: ESP32 → App
  pNotifyChar = pService->createCharacteristic(
      CHAR_NOTIFY_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pNotifyChar->addDescriptor(new BLE2902());

  // Write characteristic: App → ESP32
  pWriteChar = pService->createCharacteristic(
      CHAR_WRITE_UUID, BLECharacteristic::PROPERTY_WRITE);
  pWriteChar->setCallbacks(new WriteCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising as '" BLE_DEVICE_NAME "'.");
}

// ─── Send notification to app
// ─────────────────────────────────────────────────
void sendBLENotification(String message) {
  if (!bleConnected) {
    Serial.println("[BLE] Cannot send — not connected.");
    return;
  }
  Serial.print("[BLE] Sending: ");
  Serial.println(message);
  pNotifyChar->setValue(message.c_str());
  pNotifyChar->notify();
}

// ─── Skip BLE (short press during connection phase)
// ───────────────────────────
void skipBLEConnection() {
  bleSkipped = true;
  Serial.println("[BLE] Skipped by user.");
  playAudio(AUDIO_BLE_SKIPPED);
  waitForAudioFinish();
  currentState = STATE_RUNNING;
}

// ─── BLE connection handler — called from STATE_BLE_CONNECTING in loop()
// ────── Non-blocking: uses millis() timing so sonar/button still run in
// parallel. Flow:
//   1. Play "connecting" audio
//   2. Wait up to BLE_CONNECT_TIMEOUT_MS for phone to connect
//   3. If connected → play success → STATE_RUNNING
//   4. If timeout  → play retry   → repeat from 1
//   5. Short button press at any point → skipBLEConnection()
//
// Note: Button handling for skip is called directly in the
//       STATE_BLE_CONNECTING case in loop() — no need to call it here.

#define BLE_CONNECT_TIMEOUT_MS 10000 // 10s per attempt before retry audio

static unsigned long _bleAttemptStartMs = 0;
static bool _bleAudioPlayed = false;

void handleBLEConnection() {
  // First call: start the attempt
  if (_bleAttemptStartMs == 0) {
    _bleAttemptStartMs = millis();
    _bleAudioPlayed = false;
  }

  // Play "connecting" audio once per attempt
  if (!_bleAudioPlayed && !isAudioPlaying()) {
    playAudio(AUDIO_BLE_CONNECTING);
    _bleAudioPlayed = true;
  }

  // Connected!
  if (bleConnected) {
    Serial.println("[BLE] Connection established.");
    playAudio(AUDIO_BLE_CONNECTED);
    waitForAudioFinish();
    _bleAttemptStartMs = 0;
    _bleAudioPlayed = false;
    currentState = STATE_RUNNING;
    return;
  }

  // Timeout — play retry audio and reset for next attempt
  if (millis() - _bleAttemptStartMs >= BLE_CONNECT_TIMEOUT_MS) {
    Serial.println("[BLE] Connection timeout. Retrying...");
    waitForAudioFinish(); // Let "connecting" audio finish first
    playAudio(AUDIO_BLE_RETRY);
    waitForAudioFinish();
    _bleAttemptStartMs = 0; // Reset — next loop iteration starts fresh
    _bleAudioPlayed = false;
  }
}

// ─── Mid-session reconnect handler — called from STATE_RUNNING in loop()
// ────── When the phone disconnects unexpectedly:
//   1. Stop everything (vibration, audio)
//   2. Play "connection lost" audio
//   3. Enter a tight reconnect loop (same logic as initial connect)
//   4. On reconnect → resume STATE_RUNNING
//   5. Short press → skip, resume STATE_RUNNING without BLE
//
void handleMidSessionReconnect() {
  stopVibration();
  stopAudio();

  Serial.println("[BLE] Mid-session disconnect. Pausing to reconnect...");
  playAudio(AUDIO_BLE_LOST);
  waitForAudioFinish();

  // Re-enter connection loop by resetting to BLE_CONNECTING state
  pendingReconnect = false;
  _bleAttemptStartMs = 0;
  _bleAudioPlayed = false;
  currentState = STATE_BLE_CONNECTING;
}
