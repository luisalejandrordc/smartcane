// ═══════════════════════════════════════════════════════════════════════════════
//  BLEManager.ts — BLE scanning, connection, notification handling
// ═══════════════════════════════════════════════════════════════════════════════

import { BleManager, Device, Subscription } from "react-native-ble-plx";
import { Platform, PermissionsAndroid } from "react-native";
import { Buffer } from "buffer";
import {
  BLE_DEVICE_NAME,
  SERVICE_UUID,
  CHAR_NOTIFY_UUID,
  CHAR_WRITE_UUID,
  parseSOSMessage,
  SOSCoordinates,
} from "./BLEConstants";
import { ConnectionStatus } from "../types";

// ─── Callbacks the rest of the app can register ───────────────────────────────
export interface BLECallbacks {
  onConnectionStatusChange: (status: ConnectionStatus) => void;
  onSOSReceived: (coords: SOSCoordinates) => void;
}

class BLEManagerService {
  private manager: BleManager;
  private device: Device | null = null;
  private notifySubscription: Subscription | null = null;
  private callbacks: BLECallbacks | null = null;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  private isReconnecting: boolean = false;

  constructor() {
    this.manager = new BleManager();
  }

  // ─── Register callbacks ─────────────────────────────────────────────────────
  setCallbacks(callbacks: BLECallbacks) {
    this.callbacks = callbacks;
  }

  // ─── Android permission request ─────────────────────────────────────────────
  async requestPermissions(): Promise<boolean> {
    if (Platform.OS === "android") {
      const granted = await PermissionsAndroid.requestMultiple([
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      ]);
      return Object.values(granted).every(
        (v) => v === PermissionsAndroid.RESULTS.GRANTED,
      );
    }
    // iOS permissions are handled via app.json info.plist
    return true;
  }

  // ─── Start scanning for SmartCane ───────────────────────────────────────────
  async startScan(): Promise<void> {
    const hasPermission = await this.requestPermissions();
    if (!hasPermission) {
      console.error("[BLE] Permissions denied.");
      this.callbacks?.onConnectionStatusChange("error");
      return;
    }

    this.callbacks?.onConnectionStatusChange("scanning");
    console.log("[BLE] Scanning for", BLE_DEVICE_NAME);

    this.manager.startDeviceScan(
      [SERVICE_UUID], // Filter by our service UUID — faster and battery-friendly
      null,
      async (error, device) => {
        if (error) {
          console.error("[BLE] Scan error:", error.message);
          this.callbacks?.onConnectionStatusChange("error");
          return;
        }

        if (device?.name === BLE_DEVICE_NAME) {
          this.manager.stopDeviceScan();
          await this.connectToDevice(device);
        }
      },
    );
  }

  // ─── Connect to device ──────────────────────────────────────────────────────
  async connectToDevice(device: Device): Promise<void> {
    try {
      this.callbacks?.onConnectionStatusChange("connecting");
      console.log("[BLE] Connecting to", device.name);

      this.device = await device.connect();
      await this.device.discoverAllServicesAndCharacteristics();

      // Subscribe to disconnect events
      this.device.onDisconnected((error, disconnectedDevice) => {
        console.log("[BLE] Device disconnected:", disconnectedDevice?.name);
        this.callbacks?.onConnectionStatusChange("disconnected");
        this.notifySubscription?.remove();
        this.notifySubscription = null;
        this.scheduleReconnect();
      });

      // Subscribe to notifications from ESP32
      this.notifySubscription = this.device.monitorCharacteristicForService(
        SERVICE_UUID,
        CHAR_NOTIFY_UUID,
        (error, characteristic) => {
          if (error) {
            console.error("[BLE] Notification error:", error.message);
            return;
          }
          if (characteristic?.value) {
            const message = Buffer.from(
              characteristic.value,
              "base64",
            ).toString("utf-8");
            console.log("[BLE] Received:", message);
            this.handleIncomingMessage(message);
          }
        },
      );

      this.callbacks?.onConnectionStatusChange("connected");
      this.isReconnecting = false;
      console.log("[BLE] Connected and subscribed.");
    } catch (error: any) {
      console.error("[BLE] Connection failed:", error.message);
      this.callbacks?.onConnectionStatusChange("error");
      this.scheduleReconnect();
    }
  }

  // ─── Handle incoming BLE message from ESP32 ─────────────────────────────────
  private handleIncomingMessage(message: string): void {
    const coords = parseSOSMessage(message);
    if (coords) {
      console.log("[BLE] SOS received:", coords);
      this.callbacks?.onSOSReceived(coords);
    }
  }

  // ─── Send message to ESP32 ──────────────────────────────────────────────────
  async sendMessage(message: string): Promise<void> {
    if (!this.device) {
      console.error("[BLE] Cannot send — not connected.");
      return;
    }
    try {
      const encoded = Buffer.from(message, "utf-8").toString("base64");
      await this.device.writeCharacteristicWithResponseForService(
        SERVICE_UUID,
        CHAR_WRITE_UUID,
        encoded,
      );
      console.log("[BLE] Sent:", message);
    } catch (error: any) {
      console.error("[BLE] Send failed:", error.message);
    }
  }

  // ─── Auto-reconnect with backoff ────────────────────────────────────────────
  // Tries to reconnect every 5 seconds after a disconnect.
  private scheduleReconnect(): void {
    if (this.isReconnecting) return;
    this.isReconnecting = true;

    console.log("[BLE] Scheduling reconnect in 5s...");
    this.reconnectTimer = setTimeout(async () => {
      console.log("[BLE] Attempting reconnect...");
      await this.startScan();
    }, 5000);
  }

  // ─── Disconnect cleanly ─────────────────────────────────────────────────────
  async disconnect(): Promise<void> {
    if (this.reconnectTimer) clearTimeout(this.reconnectTimer);
    this.notifySubscription?.remove();
    await this.device?.cancelConnection();
    this.device = null;
    this.callbacks?.onConnectionStatusChange("disconnected");
  }

  // ─── Check connection status ────────────────────────────────────────────────
  isConnected(): boolean {
    return this.device !== null;
  }
}

// Export as singleton — one BLE manager for the whole app
export const bleManager = new BLEManagerService();
