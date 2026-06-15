#pragma once

enum SystemState { STATE_BOOT, STATE_BLE_CONNECTING, STATE_RUNNING, STATE_SOS };

extern SystemState currentState;
