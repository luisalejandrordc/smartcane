import React, { useEffect, useState } from "react";
import {
  View,
  Text,
  StyleSheet,
  TextInput,
  ScrollView,
  Linking,
  Alert,
} from "react-native";
import { loadTelegramConfig, saveTelegramConfig } from "../storage/Storage";
import { Card } from "../components/Card";
import { PrimaryButton } from "../components/PrimaryButton";
import { colors, spacing, fontSize, fontWeight } from "../theme";

export function TelegramScreen() {
  const [botToken, setBotToken] = useState("");
  const [saved, setSaved] = useState(false);
  const [testing, setTesting] = useState(false);

  useEffect(() => {
    loadTelegramConfig().then((c) => setBotToken(c.botToken));
  }, []);

  const handleSave = async () => {
    await saveTelegramConfig({ botToken: botToken.trim() });
    setSaved(true);
    setTimeout(() => setSaved(false), 2000);
  };

  const handleTest = async () => {
    if (!botToken.trim()) {
      Alert.alert("No token", "Enter your bot token first.");
      return;
    }
    setTesting(true);
    try {
      const res = await fetch(
        `https://api.telegram.org/bot${botToken.trim()}/getMe`,
      );
      const data = await res.json();
      if (data.ok) {
        Alert.alert("Token valid ✓", `Connected as @${data.result.username}`);
      } else {
        Alert.alert("Invalid token", data.description);
      }
    } catch {
      Alert.alert(
        "Network error",
        "Could not reach Telegram. Check your connection.",
      );
    }
    setTesting(false);
  };

  const openBotFather = () => Linking.openURL("https://t.me/BotFather");

  const openGetUpdates = () => {
    if (!botToken.trim()) {
      Alert.alert("No token", "Enter your bot token first.");
      return;
    }
    Linking.openURL(
      `https://api.telegram.org/bot${botToken.trim()}/getUpdates`,
    );
  };

  return (
    <ScrollView
      style={styles.container}
      contentContainerStyle={styles.content}
      keyboardShouldPersistTaps="handled"
    >
      <Text style={styles.pageTitle}>Telegram Setup</Text>
      <Text style={styles.pageSubtitle}>
        SmartCane sends SOS alerts through a Telegram bot you own. Setup takes
        about 3 minutes.
      </Text>

      {/* ── Step 1 ── */}
      <Card style={styles.stepCard}>
        <Text style={styles.stepNumber}>STEP 1</Text>
        <Text style={styles.stepTitle}>Create your bot</Text>
        <Text style={styles.stepBody}>
          Open BotFather on Telegram, send /newbot, and follow the prompts.
          You'll receive a token that looks like{" "}
          <Text style={styles.mono}>7123456789:AAF...</Text>
        </Text>
        <PrimaryButton
          label="Open BotFather"
          onPress={openBotFather}
          variant="ghost"
          style={styles.stepButton}
        />
      </Card>

      {/* ── Step 2 ── */}
      <Card style={styles.stepCard}>
        <Text style={styles.stepNumber}>STEP 2</Text>
        <Text style={styles.stepTitle}>Enter your bot token</Text>

        <Text style={styles.fieldLabel}>BOT TOKEN</Text>
        <TextInput
          style={styles.input}
          value={botToken}
          onChangeText={(v) => setBotToken(v)}
          placeholder="7123456789:AAFxxxxxx..."
          placeholderTextColor={colors.textMuted}
          autoCapitalize="none"
          autoCorrect={false}
        />

        <View style={styles.tokenActions}>
          <PrimaryButton
            label={saved ? "Saved ✓" : "Save token"}
            onPress={handleSave}
            disabled={saved}
            style={{ flex: 1, marginRight: spacing.sm }}
          />
          <PrimaryButton
            label="Test token"
            onPress={handleTest}
            loading={testing}
            variant="ghost"
            style={{ flex: 1 }}
          />
        </View>
      </Card>

      {/* ── Step 3 ── */}
      <Card style={styles.stepCard}>
        <Text style={styles.stepNumber}>STEP 3</Text>
        <Text style={styles.stepTitle}>Get your chat ID</Text>
        <Text style={styles.stepBody}>
          Message your bot on Telegram (send anything to start a chat), then tap
          below to open the getUpdates page. Find the number next to{" "}
          <Text style={styles.mono}>"chat":{"{"}"id":</Text> and copy it into
          the contact's Telegram ID field.
        </Text>
        <PrimaryButton
          label="Open getUpdates"
          onPress={openGetUpdates}
          variant="ghost"
          style={styles.stepButton}
        />
      </Card>

      <Text style={styles.note}>
        Each emergency contact needs to message your bot once to start a chat
        before alerts can be delivered to them.
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
  stepCard: {
    marginBottom: spacing.md,
  },
  stepNumber: {
    fontSize: fontSize.xs,
    fontWeight: fontWeight.bold,
    color: colors.accent,
    letterSpacing: 2,
    marginBottom: spacing.xs,
  },
  stepTitle: {
    fontSize: fontSize.lg,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
    marginBottom: spacing.sm,
  },
  stepBody: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    lineHeight: 20,
    marginBottom: spacing.md,
  },
  mono: {
    fontFamily: "Courier New",
    color: colors.accent,
    backgroundColor: colors.background,
  },
  fieldLabel: {
    fontSize: fontSize.xs,
    fontWeight: fontWeight.bold,
    color: colors.textMuted,
    letterSpacing: 1.5,
    marginBottom: spacing.xs,
  },
  input: {
    backgroundColor: colors.background,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: colors.border,
    color: colors.textPrimary,
    fontSize: fontSize.md,
    padding: spacing.md,
    marginBottom: spacing.md,
  },
  tokenActions: {
    flexDirection: "row",
  },
  stepButton: {
    alignSelf: "flex-start",
  },
  note: {
    fontSize: fontSize.sm,
    color: colors.textMuted,
    lineHeight: 20,
    textAlign: "center",
    marginTop: spacing.md,
  },
});
