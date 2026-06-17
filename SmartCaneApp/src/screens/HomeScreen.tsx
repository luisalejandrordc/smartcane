import React, { useEffect, useRef } from "react";
import {
  View,
  Text,
  StyleSheet,
  Animated,
  Easing,
  ScrollView,
} from "react-native";
import { colors, spacing, fontSize, fontWeight } from "../theme";
import { ConnectionStatus, SOSStatus } from "../types";

interface Props {
  connectionStatus: ConnectionStatus;
  sosStatus: SOSStatus;
  lastLocation: string | null;
}

// ─── Status copy ──────────────────────────────────────────────────────────────
const STATUS_LABEL: Record<ConnectionStatus, string> = {
  disconnected: "Not connected",
  scanning: "Scanning...",
  connecting: "Connecting...",
  connected: "Connected",
  error: "Connection error",
};

const STATUS_COLOR: Record<ConnectionStatus, string> = {
  disconnected: colors.textMuted,
  scanning: colors.accentDim,
  connecting: colors.accent,
  connected: colors.accent,
  error: colors.alert,
};

const SOS_LABEL: Record<SOSStatus, string> = {
  idle: "",
  receiving: "Receiving location...",
  sending: "Sending alert...",
  success: "Alert sent successfully",
  failed: "Alert failed to send",
};

export function HomeScreen({
  connectionStatus,
  sosStatus,
  lastLocation,
}: Props) {
  // ── Breathing ring animation ───────────────────────────────────────────────
  const pulse = useRef(new Animated.Value(0)).current;

  useEffect(() => {
    if (connectionStatus === "connected") {
      Animated.loop(
        Animated.sequence([
          Animated.timing(pulse, {
            toValue: 1,
            duration: 2000,
            easing: Easing.inOut(Easing.ease),
            useNativeDriver: true,
          }),
          Animated.timing(pulse, {
            toValue: 0,
            duration: 2000,
            easing: Easing.inOut(Easing.ease),
            useNativeDriver: true,
          }),
        ]),
      ).start();
    } else {
      pulse.stopAnimation();
      pulse.setValue(0);
    }
  }, [connectionStatus]);

  const ringScale = pulse.interpolate({
    inputRange: [0, 1],
    outputRange: [1, 1.18],
  });
  const ringOpacity = pulse.interpolate({
    inputRange: [0, 1],
    outputRange: [0.6, 0.15],
  });
  const ringColor = STATUS_COLOR[connectionStatus];

  return (
    <ScrollView
      style={styles.container}
      contentContainerStyle={styles.content}
      showsVerticalScrollIndicator={false}
    >
      {/* ── Hero: breathing ring ── */}
      <View style={styles.heroArea}>
        {/* Outer breathing ring */}
        <Animated.View
          style={[
            styles.ring,
            {
              borderColor: ringColor,
              opacity: ringOpacity,
              transform: [{ scale: ringScale }],
            },
          ]}
        />

        {/* Inner status circle */}
        <View style={[styles.circle, { borderColor: ringColor }]}>
          <Text style={[styles.circleIcon, { color: ringColor }]}>
            {connectionStatus === "connected" ? "◉" : "○"}
          </Text>
        </View>

        <Text style={[styles.statusLabel, { color: ringColor }]}>
          {STATUS_LABEL[connectionStatus]}
        </Text>
      </View>

      {/* ── Device info ── */}
      <View style={styles.infoSection}>
        <Text style={styles.sectionLabel}>DEVICE</Text>
        <View style={styles.infoRow}>
          <Text style={styles.infoKey}>Name</Text>
          <Text style={styles.infoValue}>SmartCane</Text>
        </View>
        <View style={styles.divider} />
        <View style={styles.infoRow}>
          <Text style={styles.infoKey}>Protocol</Text>
          <Text style={styles.infoValue}>Bluetooth LE</Text>
        </View>
        <View style={styles.divider} />
        <View style={styles.infoRow}>
          <Text style={styles.infoKey}>Background mode</Text>
          <Text style={[styles.infoValue, { color: colors.success }]}>
            Active
          </Text>
        </View>
      </View>

      {/* ── SOS status (only shown when not idle) ── */}
      {sosStatus !== "idle" && (
        <View
          style={[
            styles.sosCard,
            sosStatus === "success" && { borderColor: colors.success },
            sosStatus === "failed" && { borderColor: colors.alert },
          ]}
        >
          <Text
            style={[
              styles.sosStatus,
              sosStatus === "success" && { color: colors.success },
              sosStatus === "failed" && { color: colors.alert },
            ]}
          >
            {SOS_LABEL[sosStatus]}
          </Text>

          {lastLocation && sosStatus === "success" && (
            <Text style={styles.sosLocation} numberOfLines={2}>
              {lastLocation}
            </Text>
          )}
        </View>
      )}

      {/* ── Footer note ── */}
      <Text style={styles.footerNote}>
        Keep this app running in the background for SOS alerts to work.
      </Text>
    </ScrollView>
  );
}

const CIRCLE_SIZE = 140;
const RING_SIZE = CIRCLE_SIZE + 48;

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.background,
  },
  content: {
    paddingHorizontal: spacing.lg,
    paddingTop: spacing.xxl,
    paddingBottom: spacing.xxl,
  },
  heroArea: {
    alignItems: "center",
    marginBottom: spacing.xxl,
  },
  ring: {
    position: "absolute",
    width: RING_SIZE,
    height: RING_SIZE,
    borderRadius: RING_SIZE / 2,
    borderWidth: 1.5,
    top: -(RING_SIZE - CIRCLE_SIZE) / 2,
  },
  circle: {
    width: CIRCLE_SIZE,
    height: CIRCLE_SIZE,
    borderRadius: CIRCLE_SIZE / 2,
    borderWidth: 1.5,
    alignItems: "center",
    justifyContent: "center",
    backgroundColor: colors.surface,
  },
  circleIcon: {
    fontSize: 40,
    lineHeight: 48,
  },
  statusLabel: {
    marginTop: spacing.lg,
    fontSize: fontSize.lg,
    fontWeight: fontWeight.medium,
    letterSpacing: 0.5,
  },
  sectionLabel: {
    fontSize: fontSize.xs,
    fontWeight: fontWeight.bold,
    color: colors.textMuted,
    letterSpacing: 2,
    marginBottom: spacing.md,
  },
  infoSection: {
    backgroundColor: colors.surface,
    borderRadius: 12,
    borderWidth: 1,
    borderColor: colors.border,
    padding: spacing.lg,
    marginBottom: spacing.lg,
  },
  infoRow: {
    flexDirection: "row",
    justifyContent: "space-between",
    paddingVertical: spacing.sm,
  },
  infoKey: {
    fontSize: fontSize.md,
    color: colors.textSecondary,
    fontWeight: fontWeight.regular,
  },
  infoValue: {
    fontSize: fontSize.md,
    color: colors.textPrimary,
    fontWeight: fontWeight.medium,
  },
  divider: {
    height: 1,
    backgroundColor: colors.border,
  },
  sosCard: {
    backgroundColor: colors.surface,
    borderRadius: 12,
    borderWidth: 1,
    borderColor: colors.border,
    padding: spacing.lg,
    marginBottom: spacing.lg,
  },
  sosStatus: {
    fontSize: fontSize.md,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
    marginBottom: spacing.xs,
  },
  sosLocation: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    marginTop: spacing.xs,
  },
  footerNote: {
    fontSize: fontSize.sm,
    color: colors.textMuted,
    textAlign: "center",
    lineHeight: 20,
    marginTop: spacing.lg,
  },
});
