import { Buffer } from "buffer";
(globalThis as any).Buffer = Buffer;

import "react-native-gesture-handler";
import React, { useEffect, useState } from "react";
import { NavigationContainer } from "@react-navigation/native";
import { createBottomTabNavigator } from "@react-navigation/bottom-tabs";
import { Text, AppState } from "react-native";

import { bleManager } from "./src/ble/BLEManager";
import { sendSOSAlerts } from "./src/alerts/AlertManager";
import { buildMapsLink } from "./src/ble/BLEConstants";
import { loadConfig } from "./src/storage/Storage";
import { buildConfigMessage } from "./src/ble/BLEConstants";
import { ConnectionStatus, SOSStatus } from "./src/types";

import { HomeScreen } from "./src/screens/HomeScreen";
import { ContactsScreen } from "./src/screens/ContactsScreen";
import { TelegramScreen } from "./src/screens/TelegramScreen";
import { SettingsScreen } from "./src/screens/SettingsScreen";
import { colors, fontSize } from "./src/theme";

const Tab = createBottomTabNavigator();

export default function App() {
  const [connectionStatus, setConnectionStatus] =
    useState<ConnectionStatus>("disconnected");
  const [sosStatus, setSOSStatus] = useState<SOSStatus>("idle");
  const [lastLocation, setLastLocation] = useState<string | null>(null);

  useEffect(() => {
    bleManager.setCallbacks({
      onConnectionStatusChange: async (status) => {
        setConnectionStatus(status);

        // Re-send saved config whenever we connect
        if (status === "connected") {
          const config = await loadConfig();
          const msg = buildConfigMessage(
            config.vibrationLevel,
            config.sensitivityLevel,
            config.buzzerEnabled,
          );
          await bleManager.sendMessage(msg);
        }
      },

      onSOSReceived: async (coords) => {
        setSOSStatus("receiving");
        setLastLocation(buildMapsLink(coords.lat, coords.lon));

        try {
          setSOSStatus("sending");
          const result = await sendSOSAlerts(coords.lat, coords.lon);

          if (result.success) {
            setSOSStatus("success");
            await bleManager.sendMessage("OK");
          } else {
            setSOSStatus("failed");
            await bleManager.sendMessage("FAIL");
          }
        } catch {
          setSOSStatus("failed");
          await bleManager.sendMessage("FAIL");
        }

        // Reset SOS status after 10 seconds
        setTimeout(() => setSOSStatus("idle"), 10000);
      },
    });

    // Auto-start scanning on app launch
    bleManager.startScan();

    // Re-scan when app comes to foreground after being backgrounded
    const sub = AppState.addEventListener("change", (state) => {
      if (state === "active" && connectionStatus === "disconnected") {
        bleManager.startScan();
      }
    });

    return () => {
      sub.remove();
      bleManager.disconnect();
    };
  }, []);

  // Tab bar icon helper (text-based, no icon library needed)
  const icon =
    (label: string) =>
    ({ color }: { color: string }) => (
      <Text style={{ fontSize: fontSize.lg, color }}>{label}</Text>
    );

  return (
    <NavigationContainer>
      <Tab.Navigator
        screenOptions={{
          headerStyle: { backgroundColor: colors.background },
          headerTintColor: colors.textPrimary,
          headerTitleStyle: { fontWeight: "600" },
          tabBarStyle: {
            backgroundColor: colors.surface,
            borderTopColor: colors.border,
          },
          tabBarActiveTintColor: colors.accent,
          tabBarInactiveTintColor: colors.textMuted,
        }}
      >
        <Tab.Screen name="Home" options={{ tabBarIcon: icon("◉") }}>
          {() => (
            <HomeScreen
              connectionStatus={connectionStatus}
              sosStatus={sosStatus}
              lastLocation={lastLocation}
            />
          )}
        </Tab.Screen>

        <Tab.Screen
          name="Contacts"
          component={ContactsScreen}
          options={{ tabBarIcon: icon("☎") }}
        />

        <Tab.Screen
          name="Telegram"
          component={TelegramScreen}
          options={{ tabBarIcon: icon("✈") }}
        />

        <Tab.Screen
          name="Settings"
          component={SettingsScreen}
          options={{ tabBarIcon: icon("⚙") }}
        />
      </Tab.Navigator>
    </NavigationContainer>
  );
}
