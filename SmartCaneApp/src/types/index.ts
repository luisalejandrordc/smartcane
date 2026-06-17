// ═══════════════════════════════════════════════════════════════════════════════
//  types/index.ts — Shared TypeScript types
// ═══════════════════════════════════════════════════════════════════════════════

export type AlertMethod = "telegram" | "sms" | "both";

export interface EmergencyContact {
  id: string;
  name: string;
  phone: string; // Used for SMS
  telegramId: string; // Telegram chat_id
  alertMethod: AlertMethod;
}

export interface AppConfig {
  vibrationLevel: 1 | 2 | 3;
  sensitivityLevel: 1 | 2 | 3;
  buzzerEnabled: boolean;
}

export interface TelegramConfig {
  botToken: string;
}

export type ConnectionStatus =
  | "disconnected"
  | "scanning"
  | "connecting"
  | "connected"
  | "error";

export type SOSStatus =
  | "idle"
  | "receiving" // Got SOS notification, parsing coordinates
  | "sending" // Sending alert via Telegram/SMS
  | "success"
  | "failed";
