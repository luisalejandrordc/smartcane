#pragma once

// System mode — tracks what the system is currently doing
enum SystemState { STATE_BOOT, STATE_BLE_CONNECTING, STATE_RUNNING, STATE_SOS };

extern SystemState currentState;
