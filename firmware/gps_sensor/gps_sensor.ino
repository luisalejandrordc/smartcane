// ═══════════════════════════════════════════════════════════════════════════════
//  GPS Strict Benchmark Test (Matches Production Logic)
// ═══════════════════════════════════════════════════════════════════════════════

#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// ─── Pin Definitions ───────────────────────────────────────────────────────────
#define PIN_GPS_RX    16
#define PIN_GPS_TX    17
#define GPS_BAUD_RATE 9600

// ─── Global Objects ────────────────────────────────────────────────────────────
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

unsigned long powerOnTime = 0;
bool fixAcquired = false;
unsigned long lastStatusPrintMs = 0;

void setup() {
  // Record start time immediately
  powerOnTime = millis();

  Serial.begin(115200);
  delay(1000); 
  
  Serial.println("\n=== GPS Strict Fix Benchmark ===");
  Serial.println("[SYSTEM] Initializing UART2...");

  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  
  Serial.println("[SYSTEM] Ready. Waiting for STRICT fix.");
  Serial.println("[SYSTEM] Requires: location valid, updated, Sats >= 3, HDOP < 3.00");
  Serial.println("═════════════════════════════════════════════════════════════════");
}

void loop() {
  // 1. Feed the parser
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // 2. Define the exact same strict conditions used in your gps.h file
  bool strictFix = gps.location.isValid() && 
                   gps.location.isUpdated() &&
                   gps.hdop.isValid() && 
                   gps.hdop.value() < 300 && 
                   gps.satellites.isValid() && 
                   gps.satellites.value() >= 3;

  if (strictFix) {
    // If this is the very first time we hit the strict criteria, print the time
    if (!fixAcquired) {
      unsigned long timeToFix = millis() - powerOnTime;
      
      Serial.println("\n✅ STRICT FIX ACQUIRED!");
      Serial.print("⏱️  Time to strict fix: ");
      Serial.print(timeToFix / 1000.0);
      Serial.println(" seconds");
      Serial.println("═════════════════════════════════════════════════════\n");
      
      fixAcquired = true;
    }

    // Continuously print the valid coordinates
    Serial.print("Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print("  |  Lon: ");
    Serial.print(gps.location.lng(), 6);
    Serial.print("  |  Sats: ");
    Serial.print(gps.satellites.value());
    Serial.print("  |  HDOP: ");
    Serial.println(gps.hdop.value() / 100.0);

    delay(1000); // Slow down the output

  } else if (!fixAcquired) {
    // 3. Status feedback while waiting (prints every 2 seconds)
    if (millis() - lastStatusPrintMs > 2000) {
      lastStatusPrintMs = millis();
      
      Serial.print("[WAITING] Time elapsed: ");
      Serial.print((millis() - powerOnTime) / 1000);
      Serial.print("s  |  Sats in view: ");
      
      // Print current satellite count (or 0 if invalid)
      if (gps.satellites.isValid()) {
        Serial.print(gps.satellites.value());
      } else {
        Serial.print("0");
      }
      
      Serial.print("  |  Current HDOP: ");
      
      // Print current HDOP (or "N/A" if invalid)
      if (gps.hdop.isValid()) {
        Serial.println(gps.hdop.value() / 100.0);
      } else {
        Serial.println("N/A");
      }
    }
  }
}