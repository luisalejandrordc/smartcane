import { Buffer } from "buffer";
(globalThis as any).Buffer = Buffer;

import React, { useEffect, useState } from "react";
import { View, Text, StyleSheet, TouchableOpacity } from "react-native";
import { bleManager } from "./src/ble/BLEManager";
import { buildConfigMessage, buildMapsLink } from "./src/ble/BLEConstants";
import { ConnectionStatus, SOSStatus } from "./src/types";

export default function App() {
  const [connectionStatus, setConnectionStatus] =
    useState<ConnectionStatus>("disconnected");
  const [sosStatus, setSOSStatus] = useState<SOSStatus>("idle");
  const [lastLocation, setLastLocation] = useState<string | null>(null);

  useEffect(() => {
    bleManager.setCallbacks({
      onConnectionStatusChange: (status) => {
        console.log("Connection status:", status);
        setConnectionStatus(status);
      },

      onSOSReceived: async (coords) => {
        console.log("SOS received:", coords);
        setSOSStatus("receiving");
        setLastLocation(buildMapsLink(coords.lat, coords.lon));

        // Phase 4 will handle actual alert sending here.
        // For now, just respond OK so the ESP32 gets feedback.
        setSOSStatus("sending");
        await bleManager.sendMessage("OK");
        setSOSStatus("success");
      },
    });

    // Auto-start scanning on app launch
    bleManager.startScan();

    return () => {
      bleManager.disconnect();
    };
  }, []);

  const sendTestConfig = async () => {
    const msg = buildConfigMessage(2, 2, false);
    await bleManager.sendMessage(msg);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>SmartCane</Text>

      <View style={styles.statusRow}>
        <View
          style={[
            styles.statusDot,
            {
              backgroundColor:
                connectionStatus === "connected" ? "#4CAF50" : "#F44336",
            },
          ]}
        />
        <Text style={styles.statusText}>{connectionStatus}</Text>
      </View>

      <Text style={styles.label}>SOS Status: {sosStatus}</Text>

      {lastLocation && (
        <Text style={styles.label} numberOfLines={2}>
          Last location: {lastLocation}
        </Text>
      )}

      <TouchableOpacity style={styles.button} onPress={sendTestConfig}>
        <Text style={styles.buttonText}>Send Test Config (2,2,off)</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#121212",
    alignItems: "center",
    justifyContent: "center",
    padding: 24,
  },
  title: {
    fontSize: 32,
    fontWeight: "bold",
    color: "#FFFFFF",
    marginBottom: 32,
  },
  statusRow: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 16,
  },
  statusDot: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginRight: 8,
  },
  statusText: {
    fontSize: 18,
    color: "#FFFFFF",
    textTransform: "capitalize",
  },
  label: {
    fontSize: 14,
    color: "#AAAAAA",
    marginBottom: 12,
    textAlign: "center",
  },
  button: {
    marginTop: 32,
    backgroundColor: "#1E88E5",
    paddingVertical: 14,
    paddingHorizontal: 28,
    borderRadius: 12,
  },
  buttonText: {
    color: "#FFFFFF",
    fontSize: 16,
    fontWeight: "600",
  },
});
