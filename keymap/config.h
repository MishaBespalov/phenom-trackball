#pragma once

// --- Vial (required by the Ergohaven core / settings system) ---
#define VIAL_KEYBOARD_UID {0x5D, 0x0D, 0xDD, 0xD9, 0xED, 0xD3, 0x45, 0x17}
#define VIAL_UNLOCK_COMBO_ROWS { 1, 1 }
#define VIAL_UNLOCK_COMBO_COLS { 4, 3 }

// Run keymap-level process_record_user before the Ergohaven core handler,
// so our custom keycodes are resolved first.
#define EH_PROCESS_RECORD_USER_FIRST

// Home-row mod timing (ported from the TOTEM config).
// TAPPING_TERM seeds the QMK-Settings default tapping term.
#define TAPPING_TERM 250
#define QUICK_TAP_TERM 175
#define PERMISSIVE_HOLD
// CHORDAL_HOLD is already enabled by the Vial settings build (-DCHORDAL_HOLD).
// Our custom get_chordal_hold() in keymap.c overrides the (now weak) default.

// Combos
#define COMBO_TERM 40
#define COMBO_ONLY_FROM_LAYER 0

// Mouse keys — scroll wheel (NAV layer wheel keys)
#define MOUSEKEY_WHEEL_DELAY 100
#define MOUSEKEY_WHEEL_INTERVAL 80
#define MOUSEKEY_WHEEL_MAX_SPEED 4
#define MOUSEKEY_WHEEL_TIME_TO_MAX 40
