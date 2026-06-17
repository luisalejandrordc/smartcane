import React, { useEffect, useState } from "react";
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TextInput,
  TouchableOpacity,
  Alert,
} from "react-native";
import { EmergencyContact, AlertMethod } from "../types";
import {
  loadContacts,
  addContact,
  removeContact,
  updateContact,
} from "../storage/Storage";
import { Card } from "../components/Card";
import { PrimaryButton } from "../components/PrimaryButton";
import { colors, spacing, fontSize, fontWeight, radius } from "../theme";

// ─── Simple UUID without external dependency ───────────────────────────────
function generateId(): string {
  return Date.now().toString(36) + Math.random().toString(36).slice(2);
}

// ─── Empty form state ──────────────────────────────────────────────────────
const EMPTY_FORM = {
  name: "",
  phone: "",
  telegramId: "",
  alertMethod: "telegram" as AlertMethod,
};

export function ContactsScreen() {
  const [contacts, setContacts] = useState<EmergencyContact[]>([]);
  const [showForm, setShowForm] = useState(false);
  const [editing, setEditing] = useState<EmergencyContact | null>(null);
  const [form, setForm] = useState(EMPTY_FORM);

  useEffect(() => {
    loadContacts().then(setContacts);
  }, []);

  // ── Save (add or update) ────────────────────────────────────────────────
  const handleSave = async () => {
    if (!form.name.trim()) {
      Alert.alert("Name required", "Please enter a contact name.");
      return;
    }
    if (form.alertMethod !== "sms" && !form.telegramId.trim()) {
      Alert.alert(
        "Telegram ID required",
        "Enter the Telegram chat ID, or switch the alert method to SMS.",
      );
      return;
    }
    if (form.alertMethod !== "telegram" && !form.phone.trim()) {
      Alert.alert(
        "Phone required",
        "Enter a phone number, or switch the alert method to Telegram.",
      );
      return;
    }

    if (editing) {
      const updated = await updateContact({ ...editing, ...form });
      setContacts(updated);
    } else {
      const updated = await addContact({ id: generateId(), ...form });
      setContacts(updated);
    }
    resetForm();
  };

  const handleEdit = (contact: EmergencyContact) => {
    setEditing(contact);
    setForm({
      name: contact.name,
      phone: contact.phone,
      telegramId: contact.telegramId,
      alertMethod: contact.alertMethod,
    });
    setShowForm(true);
  };

  const handleDelete = (contact: EmergencyContact) => {
    Alert.alert(
      "Remove contact",
      `Remove ${contact.name} from emergency contacts?`,
      [
        { text: "Cancel", style: "cancel" },
        {
          text: "Remove",
          style: "destructive",
          onPress: async () => {
            const updated = await removeContact(contact.id);
            setContacts(updated);
          },
        },
      ],
    );
  };

  const resetForm = () => {
    setForm(EMPTY_FORM);
    setEditing(null);
    setShowForm(false);
  };

  // ── Method toggle ────────────────────────────────────────────────────────
  const METHOD_OPTIONS: { value: AlertMethod; label: string }[] = [
    { value: "telegram", label: "Telegram" },
    { value: "sms", label: "SMS" },
    { value: "both", label: "Both" },
  ];

  return (
    <ScrollView
      style={styles.container}
      contentContainerStyle={styles.content}
      keyboardShouldPersistTaps="handled"
    >
      <Text style={styles.pageTitle}>Emergency Contacts</Text>
      <Text style={styles.pageSubtitle}>
        When SOS is triggered, all contacts receive an alert with the location.
      </Text>

      {/* ── Contact list ── */}
      {contacts.length === 0 && !showForm && (
        <View style={styles.emptyState}>
          <Text style={styles.emptyIcon}>☎</Text>
          <Text style={styles.emptyTitle}>No contacts yet</Text>
          <Text style={styles.emptySubtitle}>
            Add at least one emergency contact to use the SOS feature.
          </Text>
        </View>
      )}

      {contacts.map((contact) => (
        <Card key={contact.id} style={styles.contactCard}>
          <View style={styles.contactHeader}>
            <View style={styles.contactAvatar}>
              <Text style={styles.contactAvatarText}>
                {contact.name.charAt(0).toUpperCase()}
              </Text>
            </View>
            <View style={styles.contactInfo}>
              <Text style={styles.contactName}>{contact.name}</Text>
              <Text style={styles.contactMeta}>
                {contact.alertMethod.toUpperCase()} ·{" "}
                {contact.alertMethod !== "sms"
                  ? `ID: ${contact.telegramId}`
                  : contact.phone}
              </Text>
            </View>
          </View>

          <View style={styles.contactActions}>
            <TouchableOpacity
              style={styles.actionBtn}
              onPress={() => handleEdit(contact)}
            >
              <Text style={styles.actionBtnText}>Edit</Text>
            </TouchableOpacity>
            <TouchableOpacity
              style={[styles.actionBtn, styles.actionBtnDanger]}
              onPress={() => handleDelete(contact)}
            >
              <Text style={styles.actionBtnDangerText}>Remove</Text>
            </TouchableOpacity>
          </View>
        </Card>
      ))}

      {/* ── Add / edit form ── */}
      {showForm && (
        <Card elevated style={styles.formCard}>
          <Text style={styles.formTitle}>
            {editing ? "Edit contact" : "New contact"}
          </Text>

          <Text style={styles.fieldLabel}>NAME</Text>
          <TextInput
            style={styles.input}
            value={form.name}
            onChangeText={(v) => setForm((f) => ({ ...f, name: v }))}
            placeholder="Full name"
            placeholderTextColor={colors.textMuted}
          />

          <Text style={styles.fieldLabel}>ALERT METHOD</Text>
          <View style={styles.methodRow}>
            {METHOD_OPTIONS.map((opt) => (
              <TouchableOpacity
                key={opt.value}
                style={[
                  styles.methodChip,
                  form.alertMethod === opt.value && styles.methodChipActive,
                ]}
                onPress={() =>
                  setForm((f) => ({ ...f, alertMethod: opt.value }))
                }
              >
                <Text
                  style={[
                    styles.methodChipText,
                    form.alertMethod === opt.value &&
                      styles.methodChipTextActive,
                  ]}
                >
                  {opt.label}
                </Text>
              </TouchableOpacity>
            ))}
          </View>

          {(form.alertMethod === "telegram" || form.alertMethod === "both") && (
            <>
              <Text style={styles.fieldLabel}>TELEGRAM CHAT ID</Text>
              <TextInput
                style={styles.input}
                value={form.telegramId}
                onChangeText={(v) => setForm((f) => ({ ...f, telegramId: v }))}
                placeholder="e.g. 123456789"
                placeholderTextColor={colors.textMuted}
                keyboardType="numeric"
              />
            </>
          )}

          {(form.alertMethod === "sms" || form.alertMethod === "both") && (
            <>
              <Text style={styles.fieldLabel}>PHONE NUMBER</Text>
              <TextInput
                style={styles.input}
                value={form.phone}
                onChangeText={(v) => setForm((f) => ({ ...f, phone: v }))}
                placeholder="+1 555 000 0000"
                placeholderTextColor={colors.textMuted}
                keyboardType="phone-pad"
              />
            </>
          )}

          <View style={styles.formActions}>
            <PrimaryButton
              label="Save contact"
              onPress={handleSave}
              style={{ flex: 1, marginRight: spacing.sm }}
            />
            <PrimaryButton
              label="Cancel"
              onPress={resetForm}
              variant="ghost"
              style={{ flex: 1 }}
            />
          </View>
        </Card>
      )}

      {!showForm && (
        <PrimaryButton
          label="+ Add contact"
          onPress={() => setShowForm(true)}
          style={styles.addButton}
        />
      )}
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
  emptyState: {
    alignItems: "center",
    paddingVertical: spacing.xxl,
  },
  emptyIcon: {
    fontSize: 40,
    marginBottom: spacing.md,
  },
  emptyTitle: {
    fontSize: fontSize.lg,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
    marginBottom: spacing.xs,
  },
  emptySubtitle: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    textAlign: "center",
    lineHeight: 20,
  },
  contactCard: {
    marginBottom: spacing.md,
  },
  contactHeader: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: spacing.md,
  },
  contactAvatar: {
    width: 44,
    height: 44,
    borderRadius: 22,
    backgroundColor: colors.accentDim,
    alignItems: "center",
    justifyContent: "center",
    marginRight: spacing.md,
  },
  contactAvatarText: {
    fontSize: fontSize.lg,
    fontWeight: fontWeight.bold,
    color: colors.accent,
  },
  contactInfo: {
    flex: 1,
  },
  contactName: {
    fontSize: fontSize.md,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
  },
  contactMeta: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    marginTop: spacing.xs,
  },
  contactActions: {
    flexDirection: "row",
    gap: spacing.sm,
  },
  actionBtn: {
    paddingVertical: spacing.sm,
    paddingHorizontal: spacing.md,
    borderRadius: radius.sm,
    borderWidth: 1,
    borderColor: colors.border,
  },
  actionBtnText: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    fontWeight: fontWeight.medium,
  },
  actionBtnDanger: {
    borderColor: colors.alertDim,
  },
  actionBtnDangerText: {
    fontSize: fontSize.sm,
    color: colors.alert,
    fontWeight: fontWeight.medium,
  },
  formCard: {
    marginBottom: spacing.md,
  },
  formTitle: {
    fontSize: fontSize.lg,
    fontWeight: fontWeight.semibold,
    color: colors.textPrimary,
    marginBottom: spacing.lg,
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
    borderRadius: radius.sm,
    borderWidth: 1,
    borderColor: colors.border,
    color: colors.textPrimary,
    fontSize: fontSize.md,
    padding: spacing.md,
    marginBottom: spacing.lg,
  },
  methodRow: {
    flexDirection: "row",
    gap: spacing.sm,
    marginBottom: spacing.lg,
  },
  methodChip: {
    flex: 1,
    paddingVertical: spacing.sm,
    borderRadius: radius.sm,
    borderWidth: 1,
    borderColor: colors.border,
    alignItems: "center",
  },
  methodChipActive: {
    backgroundColor: colors.accentDim,
    borderColor: colors.accent,
  },
  methodChipText: {
    fontSize: fontSize.sm,
    color: colors.textSecondary,
    fontWeight: fontWeight.medium,
  },
  methodChipTextActive: {
    color: colors.accent,
  },
  formActions: {
    flexDirection: "row",
    marginTop: spacing.sm,
  },
  addButton: {
    marginTop: spacing.md,
  },
});
