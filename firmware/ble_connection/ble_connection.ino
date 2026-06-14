// ─── HARDWARE SETUP ───────────────────────────────────────────────────────────
// One leg to GPIO 15 (or change BUTTON_PIN), other leg to GND.
// No resistor needed — we're using INPUT_PULLUP.
// Install nRF Connect (by Nordic Semiconductor) on your phone — it's the standard
// tool for this. You should see SmartCane appear when scanning. Connect to it,
// find the notify characteristic, subscribe to it, then hold your button for 3
// seconds. You should see the SOS:-12.046374,-77.042793 message arrive in nRF
// Connect. You can also manually write OK or FAIL to the write characteristic
// and watch the Serial Monitor react.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ─── BLE UUIDs ────────────────────────────────────────────────────────────────
// These are custom UUIDs for our service and characteristics.
// You can keep these exact values — they just need to be consistent
// between the ESP32 firmware and the mobile app.
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHAR_NOTIFY_UUID    "12345678-1234-1234-1234-123456789ab1"  // ESP32 → App
#define CHAR_WRITE_UUID     "12345678-1234-1234-1234-123456789ab2"  // App → ESP32

// ─── Pin definitions ──────────────────────────────────────────────────────────
#define BUTTON_PIN 15   // SOS button — change to whatever GPIO you're using

// ─── Hold duration ────────────────────────────────────────────────────────────
#define HOLD_THRESHOLD_MS 3000  // 3 seconds to trigger SOS

// ─── Simulated GPS coordinates (Lima, Peru) ───────────────────────────────────
#define FAKE_LAT "-12.046374"
#define FAKE_LON "-77.042793"

// ─── BLE state ────────────────────────────────────────────────────────────────
BLEServer*          pServer         = nullptr;
BLECharacteristic*  pNotifyChar     = nullptr;
BLECharacteristic*  pWriteChar      = nullptr;
bool                deviceConnected = false;

// ─── Button state ─────────────────────────────────────────────────────────────
unsigned long buttonPressedAt = 0;
bool          buttonHeld      = false;
bool          sosTriggered    = false;  // prevents re-triggering while held

// ─── BLE Server Callbacks ─────────────────────────────────────────────────────
// These fire when a phone connects or disconnects.
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("[BLE] Phone connected.");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("[BLE] Phone disconnected. Restarting advertising...");
    // Restart advertising so the phone can reconnect
    pServer->startAdvertising();
  }
};

// ─── BLE Write Callbacks ──────────────────────────────────────────────────────
// This fires when the app writes "OK" or "FAIL" back to the ESP32.
class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String value = pChar->getValue().c_str();
    value.trim();

    Serial.print("[APP RESPONSE] ");

    if (value == "OK") {
      Serial.println("Alert sent successfully!");
      // Phase 5: trigger MP3 success audio here
    } else if (value == "FAIL") {
      Serial.println("Alert FAILED to send.");
      // Phase 5: trigger MP3 failure audio here
    } else {
      Serial.print("Unknown response: ");
      Serial.println(value);
    }
  }
};

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button connects GPIO to GND when pressed

  Serial.println("[BOOT] Starting BLE...");

  // Initialize BLE with a device name (this is what appears when scanning)
  BLEDevice::init("SmartCane");

  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Characteristic 1: ESP32 → App (NOTIFY)
  // The app subscribes to this and receives data when ESP32 calls notify()
  pNotifyChar = pService->createCharacteristic(
    CHAR_NOTIFY_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pNotifyChar->addDescriptor(new BLE2902());  // Required for notify to work

  // Characteristic 2: App → ESP32 (WRITE)
  // The app writes "OK" or "FAIL" here after handling the SOS
  pWriteChar = pService->createCharacteristic(
    CHAR_WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pWriteChar->setCallbacks(new WriteCallbacks());

  // Start the service and begin advertising
  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising as 'SmartCane'. Waiting for connection...");
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
  handleButton();
  delay(50);  // Small delay to avoid busy-looping; 50ms is fine for button polling
}

// ─── Button Hold Detection ────────────────────────────────────────────────────
void handleButton() {
  // INPUT_PULLUP means LOW = pressed, HIGH = released
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);

  if (pressed) {
    if (buttonPressedAt == 0) {
      // First moment of press — record the time
      buttonPressedAt = millis();
      sosTriggered = false;
      Serial.println("[BUTTON] Pressed. Holding...");
    }

    // Check if held long enough and hasn't fired yet this press
    if (!sosTriggered && (millis() - buttonPressedAt >= HOLD_THRESHOLD_MS)) {
      sosTriggered = true;
      triggerSOS();
    }

  } else {
    // Button released
    if (buttonPressedAt != 0 && !sosTriggered) {
      Serial.println("[BUTTON] Released too early. No SOS triggered.");
    }
    buttonPressedAt = 0;
  }
}

// ─── SOS Trigger ─────────────────────────────────────────────────────────────
void triggerSOS() {
  Serial.println("[SOS] Triggered!");

  if (!deviceConnected) {
    Serial.println("[SOS] No phone connected — cannot send alert.");
    // Phase 5: play "no connection" audio here
    return;
  }

  // Build the message: SOS:<lat>,<lon>
  String message = "SOS:";
  message += FAKE_LAT;
  message += ",";
  message += FAKE_LON;

  Serial.print("[SOS] Sending via BLE: ");
  Serial.println(message);

  pNotifyChar->setValue(message.c_str());
  pNotifyChar->notify();
}
