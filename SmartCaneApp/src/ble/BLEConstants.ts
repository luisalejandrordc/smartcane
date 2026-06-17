// ═══════════════════════════════════════════════════════════════════════════════
//  BLEConstants.ts — UUIDs and message constants
//  Must match config.h on the ESP32 exactly.
// ═══════════════════════════════════════════════════════════════════════════════

export const BLE_DEVICE_NAME = "SmartCane";
export const SERVICE_UUID = "12345678-1234-1234-1234-123456789abc";
export const CHAR_NOTIFY_UUID = "12345678-1234-1234-1234-123456789ab1";
export const CHAR_WRITE_UUID = "12345678-1234-1234-1234-123456789ab2";

// Messages sent from app to ESP32
export const MSG_OK = "OK";
export const MSG_FAIL = "FAIL";

// Config message builder
// Format: CFG:<vibration>,<sensitivity>,<buzzer>
export function buildConfigMessage(
  vibration: 1 | 2 | 3,
  sensitivity: 1 | 2 | 3,
  buzzer: boolean,
): string {
  return `CFG:${vibration},${sensitivity},${buzzer ? 1 : 0}`;
}

// SOS message parser
// Format: SOS:<lat>,<lon>
export interface SOSCoordinates {
  lat: number;
  lon: number;
}

export function parseSOSMessage(message: string): SOSCoordinates | null {
  if (!message.startsWith("SOS:")) return null;

  const payload = message.substring(4);
  const parts = payload.split(",");

  if (parts.length !== 2) return null;

  const lat = parseFloat(parts[0]);
  const lon = parseFloat(parts[1]);

  if (isNaN(lat) || isNaN(lon)) return null;

  return { lat, lon };
}

// Google Maps link builder
export function buildMapsLink(lat: number, lon: number): string {
  return `https://maps.google.com/?q=${lat},${lon}`;
}
