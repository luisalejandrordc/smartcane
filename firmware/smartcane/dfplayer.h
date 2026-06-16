// ═══════════════════════════════════════════════════════════════════════════════
//  dfplayer.h — DFPlayer Mini Audio Feedback System
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once
#include "config.h"
#include <DFRobotDFPlayerMini.h>

// ─── Hardware Serial for DFPlayer ─────────────────────────────────────────────
// We use Serial1 (ESP32 has 3 hardware UARTs). 
// Serial0 = USB debug, Serial1 = DFPlayer, Serial2 = GPS
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfPlayer;

// ─── Internal state ────────────────────────────────────────────────────────────
bool dfPlayerReady = false;
bool audioPlaying = false;
int currentTrack = 0;
unsigned long audioStartMs = 0;

// Approximate durations per track in milliseconds.
// Update these once you've recorded your actual audio files.
// Better to be slightly over than under — it just adds a small pause.
const int AUDIO_DURATIONS_MS[] = {
    0,    // index 0 unused
    3700, // 01 - Welcome
    2800, // 02 - BLE connecting
    2600, // 03 - BLE connected
    3200, // 04 - BLE retry
    2900, // 05 - BLE skipped
    4000, // 06 - BLE lost
    3300, // 07 - Battery 100%
    2900, // 08 - Battery 90%
    2900, // 09 - Battery 80%
    2900, // 10 - Battery 70%
    2900, // 11 - Battery 60%
    2900, // 12 - Battery 50%
    2900, // 13 - Battery 40%
    2900, // 14 - Battery 30%
    3800, // 15 - Low battery 20%
    3800, // 16 - Critical battery 10%
    4000, // 17 - SOS triggered
    4500, // 18 - GPS fail
    3000, // 19 - SOS success
    3000, // 20 - SOS fail
    3200, // 21 - Config received
};

// ─── Init ──────────────────────────────────────────────────────────────────────
void initDFPlayer() {
  // TX=PIN_DFPLAYER_TX, RX=PIN_DFPLAYER_RX
  dfSerial.begin(DFPLAYER_BAUD_RATE, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
  delay(1000); // DFPlayer needs time to initialize after power-on

  if (!dfPlayer.begin(dfSerial, true, true)) {
    DEBUG_PRINTLN("[DFPLAYER] ERROR: Could not initialize. Check wiring and SD card.");
    dfPlayerReady = false;
    return;
  }

  dfPlayer.volume(DFPLAYER_VOLUME);
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayerReady = true;
  DEBUG_PRINTLN("[DFPLAYER] Ready.");
}

// ─── Play a track by index ────────────────────────────────────────────────────
// Non-blocking — returns immediately. Use isAudioPlaying() to check status.
void playAudio(int trackIndex) {
  if (!dfPlayerReady) {
    DEBUG_PRINT("[DFPLAYER] Skipping track ");
    DEBUG_PRINT(trackIndex);
    DEBUG_PRINTLN(" — player not ready.");
    return;
  }

  if (trackIndex < 1 || trackIndex > 21) {
    DEBUG_PRINT("[DFPLAYER] Invalid track index: ");
    DEBUG_PRINTLN(trackIndex);
    return;
  }

  DEBUG_PRINT("[DFPLAYER] Playing track ");
  DEBUG_PRINTLN(trackIndex);

  dfPlayer.play(trackIndex);
  audioPlaying = true;
  currentTrack = trackIndex;
  audioStartMs = millis();
}

// ─── Check if audio is still playing (time-based) ────────────────────────────
// DFPlayer Mini's busy pin is unreliable on some modules.
// We use estimated duration instead — simpler and more robust.
bool isAudioPlaying() {
  if (!audioPlaying) {
    return false;
  }

  int duration = (currentTrack < 22) ? AUDIO_DURATIONS_MS[currentTrack] : 3000;
  if (millis() - audioStartMs >= (unsigned long)duration) {
    audioPlaying = false;
  }

  return audioPlaying;
}

// ─── Blocking wait for audio to finish ───────────────────────────────────────
// Used for critical feedback (SOS result, welcome message) where
// we must ensure the user hears the full message before continuing.
// Still calls yield() so the ESP32 watchdog doesn't reset.
void waitForAudioFinish() {
  while (isAudioPlaying()) {
    yield();
    delay(50);
  }
}

// ─── Stop audio immediately ───────────────────────────────────────────────────
void stopAudio() {
  if (dfPlayerReady) {
    dfPlayer.stop();
  }
  audioPlaying = false;
}