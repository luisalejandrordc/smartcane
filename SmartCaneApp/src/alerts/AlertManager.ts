// ═══════════════════════════════════════════════════════════════════════════════
//  AlertManager.ts — Telegram Bot + SMS alert sending
// ═══════════════════════════════════════════════════════════════════════════════

import { Linking } from "react-native";
import { EmergencyContact } from "../types";
import { buildMapsLink } from "../ble/BLEConstants";
import { loadContacts, loadTelegramConfig } from "../storage/Storage";

// ─── Result type ──────────────────────────────────────────────────────────────
export interface AlertResult {
  success: boolean;
  failures: string[]; // Contact names that failed
}

// ─── Build the alert message text ─────────────────────────────────────────────
function buildAlertText(contactName: string, lat: number, lon: number): string {
  const mapsLink = buildMapsLink(lat, lon);
  return (
    `🆘 *SOS ALERT*\n\n` +
    `${contactName} needs help!\n\n` +
    `📍 Current location:\n${mapsLink}\n\n` +
    `_Sent automatically by SmartCane_`
  );
}

// ─── Telegram: send a message via Bot API ─────────────────────────────────────
// Requires:
//   - A bot token from @BotFather
//   - The recipient's chat_id (obtained by them messaging the bot first)
//
async function sendTelegramMessage(
  botToken: string,
  chatId: string,
  text: string,
): Promise<boolean> {
  try {
    const url = `https://api.telegram.org/bot${botToken}/sendMessage`;
    const response = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        chat_id: chatId,
        text: text,
        parse_mode: "Markdown",
      }),
    });

    const data = await response.json();

    if (!response.ok || !data.ok) {
      console.error("[Telegram] API error:", data.description);
      return false;
    }

    console.log("[Telegram] Message sent to chat_id:", chatId);
    return true;
  } catch (e) {
    console.error("[Telegram] Network error:", e);
    return false;
  }
}

// ─── SMS: open native SMS app with pre-filled message ─────────────────────────
// On iOS:  opens Messages with pre-filled body (user must tap Send)
// On Android: can send silently with the right permissions — but for
//             simplicity and reliability we use the same deep-link
//             approach on both platforms for now.
//
// Note: Fully silent background SMS on iOS is not permitted by Apple.
// The deep-link approach works reliably on both platforms without
// any additional native modules.
//
async function sendSMS(phone: string, text: string): Promise<boolean> {
  try {
    // SMS deep link — works on both iOS and Android
    const separator =
      "ios" === require("react-native").Platform.OS
        ? "&" // iOS uses & for body separator
        : "?"; // Android uses ?
    const url = `sms:${phone}${separator}body=${encodeURIComponent(text)}`;

    const canOpen = await Linking.canOpenURL(url);
    if (!canOpen) {
      console.error("[SMS] Cannot open SMS URL on this device.");
      return false;
    }

    await Linking.openURL(url);
    console.log("[SMS] Opened native SMS for:", phone);

    // We can't know if the user actually sent it — return true optimistically
    return true;
  } catch (e) {
    console.error("[SMS] Error:", e);
    return false;
  }
}

// ─── Main alert dispatcher ────────────────────────────────────────────────────
// Sends alerts to all configured contacts via their chosen method.
// Returns overall success (true only if ALL contacts received alert).
//
export async function sendSOSAlerts(
  lat: number,
  lon: number,
  ownerName: string = "The user",
): Promise<AlertResult> {
  const contacts = await loadContacts();
  const telegramConfig = await loadTelegramConfig();
  const failures: string[] = [];

  if (contacts.length === 0) {
    console.warn("[Alert] No contacts configured.");
    return { success: false, failures: ["No contacts configured"] };
  }

  const alertText = buildAlertText(ownerName, lat, lon);

  // Send to each contact according to their preferred method
  for (const contact of contacts) {
    let contactSuccess = false;

    if (contact.alertMethod === "telegram" || contact.alertMethod === "both") {
      if (!telegramConfig.botToken) {
        console.warn("[Alert] Telegram bot token not configured.");
        failures.push(`${contact.name} (no bot token)`);
      } else if (!contact.telegramId) {
        console.warn("[Alert] No Telegram ID for:", contact.name);
        failures.push(`${contact.name} (no Telegram ID)`);
      } else {
        const ok = await sendTelegramMessage(
          telegramConfig.botToken,
          contact.telegramId,
          alertText,
        );
        if (ok) contactSuccess = true;
        else failures.push(`${contact.name} (Telegram failed)`);
      }
    }

    if (contact.alertMethod === "sms" || contact.alertMethod === "both") {
      if (!contact.phone) {
        console.warn("[Alert] No phone number for:", contact.name);
        failures.push(`${contact.name} (no phone number)`);
      } else {
        const ok = await sendSMS(contact.phone, alertText);
        if (ok) contactSuccess = true;
        else failures.push(`${contact.name} (SMS failed)`);
      }
    }

    if (!contactSuccess) {
      failures.push(contact.name);
    }
  }

  const success = failures.length === 0;
  console.log(`[Alert] Done. Success: ${success}. Failures:`, failures);
  return { success, failures };
}
