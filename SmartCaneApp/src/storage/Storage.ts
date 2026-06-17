// ═══════════════════════════════════════════════════════════════════════════════
//  Storage.ts — Persistent storage for contacts and configuration
// ═══════════════════════════════════════════════════════════════════════════════

import AsyncStorage from "@react-native-async-storage/async-storage";
import { EmergencyContact, AppConfig, TelegramConfig } from "../types";

// ─── Storage keys ─────────────────────────────────────────────────────────────
const KEYS = {
  CONTACTS: "smartcane:contacts",
  CONFIG: "smartcane:config",
  TELEGRAM: "smartcane:telegram",
} as const;

// ─── Default values ───────────────────────────────────────────────────────────
const DEFAULT_CONFIG: AppConfig = {
  vibrationLevel: 2,
  sensitivityLevel: 2,
  buzzerEnabled: false,
};

const DEFAULT_TELEGRAM: TelegramConfig = {
  botToken: "",
};

// ─── Contacts ─────────────────────────────────────────────────────────────────
export async function loadContacts(): Promise<EmergencyContact[]> {
  try {
    const raw = await AsyncStorage.getItem(KEYS.CONTACTS);
    return raw ? JSON.parse(raw) : [];
  } catch (e) {
    console.error("[Storage] Failed to load contacts:", e);
    return [];
  }
}

export async function saveContacts(
  contacts: EmergencyContact[],
): Promise<void> {
  try {
    await AsyncStorage.setItem(KEYS.CONTACTS, JSON.stringify(contacts));
  } catch (e) {
    console.error("[Storage] Failed to save contacts:", e);
  }
}

export async function addContact(
  contact: EmergencyContact,
): Promise<EmergencyContact[]> {
  const contacts = await loadContacts();
  const updated = [...contacts, contact];
  await saveContacts(updated);
  return updated;
}

export async function removeContact(id: string): Promise<EmergencyContact[]> {
  const contacts = await loadContacts();
  const updated = contacts.filter((c) => c.id !== id);
  await saveContacts(updated);
  return updated;
}

export async function updateContact(
  updated: EmergencyContact,
): Promise<EmergencyContact[]> {
  const contacts = await loadContacts();
  const list = contacts.map((c) => (c.id === updated.id ? updated : c));
  await saveContacts(list);
  return list;
}

// ─── App config ───────────────────────────────────────────────────────────────
export async function loadConfig(): Promise<AppConfig> {
  try {
    const raw = await AsyncStorage.getItem(KEYS.CONFIG);
    return raw ? { ...DEFAULT_CONFIG, ...JSON.parse(raw) } : DEFAULT_CONFIG;
  } catch (e) {
    console.error("[Storage] Failed to load config:", e);
    return DEFAULT_CONFIG;
  }
}

export async function saveConfig(config: AppConfig): Promise<void> {
  try {
    await AsyncStorage.setItem(KEYS.CONFIG, JSON.stringify(config));
  } catch (e) {
    console.error("[Storage] Failed to save config:", e);
  }
}

// ─── Telegram config ──────────────────────────────────────────────────────────
export async function loadTelegramConfig(): Promise<TelegramConfig> {
  try {
    const raw = await AsyncStorage.getItem(KEYS.TELEGRAM);
    return raw ? JSON.parse(raw) : DEFAULT_TELEGRAM;
  } catch (e) {
    console.error("[Storage] Failed to load Telegram config:", e);
    return DEFAULT_TELEGRAM;
  }
}

export async function saveTelegramConfig(
  config: TelegramConfig,
): Promise<void> {
  try {
    await AsyncStorage.setItem(KEYS.TELEGRAM, JSON.stringify(config));
  } catch (e) {
    console.error("[Storage] Failed to save Telegram config:", e);
  }
}
