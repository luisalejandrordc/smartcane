C++
// ═══════════════════════════════════════════════════════════════════════════════
//  state.h — System State Tracking
// ═══════════════════════════════════════════════════════════════════════════════
#pragma once

// ─── System mode ───────────────────────────────────────────────────────────────
// Tracks what the system is currently doing
enum SystemState { 
  STATE_BOOT, 
  STATE_BLE_CONNECTING, 
  STATE_RUNNING, 
  STATE_SOS 
};

// Global state variable, defined in the smartcane.ino file
extern SystemState currentState;