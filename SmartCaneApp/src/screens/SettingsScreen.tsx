import React, { useEffect, useState } from "react";
import { View, Text, StyleSheet, ScrollView, Switch } from "react-native";
import { AppConfig } from "../types";
import { loadConfig, saveConfig } from "../storage/Storage";
import { buildConfigMessage } from "../ble/BLEConstants";
import { bleManager } from "../ble/BLEManager";
import { Card } from "../components/Card";
import { PrimaryButton } from "../components/PrimaryButton";
import { colors, spacing, fontSize, fontWeight, radius } from "../theme";
import { TouchableOpacity } from "react-native";

// ─── Level selector ───────────────────────────────────────────────────────────
interface LevelSelectorProps {
  value: 1 | 2 | 3;
  onChange: (v: 1 | 2 | 3) => void;
  labels: [string, string, string];
}

function LevelSelector({ value, onChange, labels }: LevelSelectorProps) {
  return (
    <View style={levelStyles.row}>
      {([1, 2, 3] as const).map((level, i) => (
        <TouchableOpacity
          key={level}
          style={[levelStyles.chip, value === level && levelStyles.chipActive]}
          onPress={() => onChange(level)}
        >
          <Text
            style={[
              levelStyles.chipLabel,
              value === level && levelStyles.chipLabelActive,
            ]}
          >
            {labels[i]}
          </Text>
        </TouchableOpacity>
      ))}
    </View>
  );
}

const levelStyles = StyleSheet.create({
  row: {
    flexDirection: "row",
    gap: spacing.sm,
    marginTop: spacing.sm,
  },
  chip: {
    flex: 1,
    paddingVertical: spacing.sm,
    borderRadius: radius.sm,
    borderWidth: 1,
    borderColor: colors.border,
    alignItems: "center",
  },
  chipActive: {
    backgroundColor: colors.accentDim,
    borderColor: colors.accent,
  },
  chipLabel: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    fontWeight: fontWeight.medium,
  },
  chipLabelActive: {
    color: colors.accent,
  },
});

// ─── Main screen ──────────────────────────────────────────────────────────────
export function SettingsScreen() {
  const [config, setConfig] = useState<AppConfig>({
    vibrationLevel: 2,
    sensitivityLevel: 2,
    buzzerEnabled: false,
  });
  const [sending, setSending] = useState(false);
  const [sent, setSent] = useState(false);

  useEffect(() => {
    loadConfig().then(setConfig);
  }, []);

  const handleApply = async () => {
    await saveConfig(config);
    setSending(true);

    const msg = buildConfigMessage(
      config.vibrationLevel,
      config.sensitivityLevel,
      config.buzzerEnabled,
    );
    await bleManager.sendMessage(msg);

    setSending(false);
    setSent(true);
    setTimeout(() => setSent(false), 2000);
  };

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      <Text style={styles.pageTitle}>Cane Settings</Text>
      <Text style={styles.pageSubtitle}>
        Changes are sent to the cane immediately. The cane must be connected.
      </Text>

      {/* ── Vibration ── */}
      <Card style={styles.settingCard}>
        <Text style={styles.settingTitle}>Vibration intensity</Text>
        <Text style={styles.settingDesc}>
          How strong the motor vibrates when an obstacle is detected.
        </Text>
        <LevelSelector
          value={config.vibrationLevel}
          onChange={(v) => setConfig((c) => ({ ...c, vibrationLevel: v }))}
          labels={["Subtle", "Medium", "Strong"]}
        />
      </Card>

      {/* ── Sensitivity ── */}
      <Card style={styles.settingCard}>
        <Text style={styles.settingTitle}>Detection range</Text>
        <Text style={styles.settingDesc}>
          How far away the cane starts detecting obstacles.
        </Text>
        <LevelSelector
          value={config.sensitivityLevel}
          onChange={(v) => setConfig((c) => ({ ...c, sensitivityLevel: v }))}
          labels={["Far (3m)", "Medium (2m)", "Close (1.2m)"]}
        />
      </Card>

      {/* ── Buzzer ── */}
      <Card style={styles.settingCard}>
        <View style={styles.switchRow}>
          <View style={styles.switchText}>
            <Text style={styles.settingTitle}>Buzzer alert</Text>
            <Text style={styles.settingDesc}>
              Audible beep when an obstacle is within 30cm. Off by default.
            </Text>
          </View>
          <Switch
            value={config.buzzerEnabled}
            onValueChange={(v) =>
              setConfig((c) => ({ ...c, buzzerEnabled: v }))
            }
            trackColor={{
              false: colors.border,
              true: colors.accentDim,
            }}
            thumbColor={config.buzzerEnabled ? colors.accent : colors.textMuted}
          />
        </View>
      </Card>

      <PrimaryButton
        label={sent ? "Sent to cane ✓" : "Apply settings"}
        onPress={handleApply}
        loading={sending}
        disabled={sent}
        style={styles.applyButton}
      />

      <Text style={styles.note}>
        Settings are also saved locally and re-applied automatically each time
        the app connects to the cane.
      </Text>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.background,
  },
  content: {
    padding: spacing.lg,
    paddingBottom: spacing.xxl,
  },
  pageTitle: {
    fontSize: fontSize.xl,
    fontWeight: fontWeight.bold,
    color: colors.textPrimary,
    marginBottom: spacing.xs,
  },
  pageSubtitle: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    lineHeight: 20,
    marginBottom: spacing.xl,
  },
  settingCard: {
    marginBottom: spacing.md,
  },
  settingTitle: {
    fontSize: fontSize.md,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
    marginBottom: spacing.xs,
  },
  settingDesc: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    lineHeight: 20,
  },
  switchRow: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
  },
  switchText: {
    flex: 1,
    marginRight: spacing.md,
  },
  applyButton: {
    marginTop: spacing.md,
  },
  note: {
    fontSize: fontSize.sm,
    color: colors.textMuted,
    textAlign: "center",
    lineHeight: 20,
    marginTop: spacing.lg,
  },
});
