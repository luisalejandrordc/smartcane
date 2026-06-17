// ═══════════════════════════════════════════════════════════════════════════════
//  theme.ts — Design tokens
// ═══════════════════════════════════════════════════════════════════════════════

export const colors = {
  background: "#1A1714",
  surface: "#2D2A27",
  surfaceHigh: "#3D3A36",
  border: "#4A4743",
  textPrimary: "#F5F0E8",
  textSecondary: "#9A948C",
  textMuted: "#6A6460",
  accent: "#C8A96E", // Brass — used for active states, labels
  accentDim: "#7A6540", // Dimmed brass — inactive states
  alert: "#E8533A", // Red — SOS status only
  alertDim: "#7A2C1E",
  success: "#5A9E6F",
  white: "#FFFFFF",
} as const;

export const spacing = {
  xs: 4,
  sm: 8,
  md: 16,
  lg: 24,
  xl: 32,
  xxl: 48,
} as const;

export const radius = {
  sm: 8,
  md: 12,
  lg: 16,
  full: 999,
} as const;

export const fontSize = {
  xs: 11,
  sm: 13,
  md: 15,
  lg: 18,
  xl: 24,
  xxl: 32,
  hero: 48,
} as const;

export const fontWeight = {
  light: "300" as const,
  regular: "400" as const,
  medium: "500" as const,
  semibold: "600" as const,
  bold: "700" as const,
  heavy: "800" as const,
};
