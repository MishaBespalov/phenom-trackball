/*
 * Phenom Micro keymap — ported from Misha's TOTEM config.
 * Russian/English dual-layer with home-row mods, custom chordal hold,
 * custom shifted mod-taps, and CapsLock-based OS language switching.
 *
 * Phenom Micro physical layout (40 keys + 2 encoder buttons):
 *
 *      | 0 | 1 | 2 | 3 | 4 |        | 5 | 6 | 7 | 8 | 9 |
 *      |10 |11 |12 |13 |14 |        |15 |16 |17 |18 |19 |
 *      |20 |21 |22 |23 |24 |        |25 |26 |27 |28 |29 |
 *   |30|31|32|33|34|                |35|36|37|38|39|
 *                       (40 enc)  (41 enc)
 *
 * Thumb keys (LAYOUT order 30..39):
 *   left  : 30=outer .. 34=inner   (34 is the home/space thumb)
 *   right : 35=inner .. 39=outer   (35 is the home/enter thumb)
 *
 * The trackball (right half), OLED, RGB and split-pointing are handled by the
 * Ergohaven keyboard core; this keymap only defines layers + custom behaviour.
 */

#include QMK_KEYBOARD_H
#include "ergohaven.h"
#include "src/eh_pointing.h" // EH_SNP (trackball sniper mode)

// ---------------------------------------------------------------------------
// Chordal Hold — opposite hands rule for home-row mods.
// The per-key hand map (chordal_hold_layout, 10x8 matrix, L/R per half with
// '*' for thumbs) is provided by the keyboard at phenom_micro/rev1/rev1.c.
// Our custom get_chordal_hold() below reads it.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Layers
// ---------------------------------------------------------------------------

enum layers {
  _EN,
  _RU,
  _SYM_EN,
  _SYM_RU,
  _NAV,
};

// ---------------------------------------------------------------------------
// Russian keycode aliases (QWERTY positions when OS is in Russian mode)
// ---------------------------------------------------------------------------

#define RU_J KC_Q      // й
#define RU_TS KC_W     // ц
#define RU_U KC_E      // у
#define RU_K KC_R      // к
#define RU_IE KC_T     // е
#define RU_N KC_Y      // н
#define RU_G KC_U      // г
#define RU_SH KC_I     // ш
#define RU_SCH KC_O    // щ
#define RU_Z KC_P      // з
#define RU_F KC_A      // ф
#define RU_Y KC_S      // ы
#define RU_V KC_D      // в
#define RU_A KC_F      // а
#define RU_P KC_G      // п
#define RU_R KC_H      // р
#define RU_O KC_J      // о
#define RU_L KC_K      // л
#define RU_D KC_L      // д
#define RU_ZH KC_SCLN  // ж
#define RU_YA KC_Z     // я
#define RU_CH KC_X     // ч
#define RU_S KC_C      // с
#define RU_M KC_V      // м
#define RU_I KC_B      // и
#define RU_T KC_N      // т
#define RU_SS KC_M     // ь (soft sign)
#define RU_B KC_COMM   // б
#define RU_YU KC_DOT   // ю
#define RU_E KC_QUOT   // э
#define RU_HA KC_LBRC  // х
#define RU_HSG KC_RBRC // ъ (hard sign)
#define RU_IO KC_GRV   // ё

// ---------------------------------------------------------------------------
// Home-row mod aliases
// ---------------------------------------------------------------------------

// EN base layer
#define GUI_S LGUI_T(KC_S)
#define ALT_T_ LALT_T(KC_T)
#define CTL_N LCTL_T(KC_N)
#define CTL_M RCTL_T(KC_M)
#define ALT_E LALT_T(KC_E)
#define GUI_I RGUI_T(KC_I)

// RU base layer
#define GUI_RF LGUI_T(RU_F)
#define ALT_RY LALT_T(RU_Y)
#define CTL_RA LCTL_T(RU_A)
#define CTL_RO RCTL_T(RU_O)
#define ALT_RD LALT_T(RU_D)
#define GUI_RZH RGUI_T(RU_ZH)

// SYM_EN layer (keys that work with standard MT)
#define GUI_BSL LGUI_T(KC_BSLS)

// NAV layer
#define CTL_LFT RCTL_T(KC_LEFT)
#define ALT_UP LALT_T(KC_UP)
#define GUI_RGT RGUI_T(KC_RGHT)

// ---------------------------------------------------------------------------
// Custom keycodes (keymap/user range — does not collide with Ergohaven's QK_KB
// custom keycodes)
// ---------------------------------------------------------------------------

enum misha_keycodes {
  // Custom mod-taps for shifted keycodes (can't use standard MT)
  ALT_LPRN = SAFE_RANGE, // hold = LALT,  tap = (
  CTL_RPRN,              // hold = LCTRL, tap = )
  GUI_COLN,              // hold = RGUI,  tap = :
  CTL_LCBR,              // hold = RCTRL, tap = {
  ALT_RCBR,              // hold = LALT,  tap = }
  // Russian keys on SYM_RU (briefly switch OS back to RU)
  RU_IO_KEY,  // ё
  RU_HA_KEY,  // х
  RU_HSG_KEY, // ъ
  // Language toggle
  LANG_SW,
};

// Language state tracking
static bool is_russian = false;

// Require-prior-idle: mod-tap is forced to tap if pressed within this many ms
// of the previous keypress (prevents accidental mods during fast typing).
#define PRIOR_IDLE_SHIFT 150
#define PRIOR_IDLE_OTHER 150
static uint16_t last_input_time = 0;

// Minimum hold time before opposite-hand mod activates (ms).
#define HOLD_TIME_SHIFT 200
#define HOLD_TIME_OTHER 200

// State for custom mod-taps — track which key owns the active mod
static uint16_t custom_mt_timer;
static uint16_t custom_mt_keycode = KC_NO;
static uint8_t custom_mt_held_mod = 0;

// Helper: custom mod-tap handler (with require-prior-idle)
static bool handle_custom_mt(uint16_t keycode, keyrecord_t *record, uint8_t mod,
                             uint16_t tap_kc, uint16_t idle_ms) {
  if (record->event.pressed) {
    // If another custom mod-tap still has a mod registered, release it first
    if (custom_mt_held_mod != 0) {
      unregister_mods(MOD_BIT(custom_mt_held_mod));
      custom_mt_held_mod = 0;
      custom_mt_keycode = KC_NO;
    }

    // If typing was recent, skip the mod and just tap
    if (last_input_time != 0 &&
        TIMER_DIFF_16(record->event.time, last_input_time) < idle_ms) {
      tap_code16(tap_kc);
    } else {
      custom_mt_timer = timer_read();
      custom_mt_keycode = keycode;
      custom_mt_held_mod = mod;
      register_mods(MOD_BIT(mod));
    }
  } else {
    // Only the key that registered the mod may unregister it
    if (custom_mt_keycode == keycode && custom_mt_held_mod != 0) {
      unregister_mods(MOD_BIT(custom_mt_held_mod));
      if (timer_elapsed(custom_mt_timer) < TAPPING_TERM) {
        // If base language is Russian but SYM_RU layer is no longer active,
        // the OS has already switched back to Russian. Briefly flip to English
        // so the tap produces the correct ASCII symbol.
        bool need_lang_fix =
            is_russian && !layer_state_cmp(layer_state, _SYM_RU);
        if (need_lang_fix)
          tap_code(KC_CAPS);
        tap_code16(tap_kc);
        if (need_lang_fix)
          tap_code(KC_CAPS);
      }
      custom_mt_held_mod = 0;
      custom_mt_keycode = KC_NO;
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// Chordal Hold override — require-prior-idle for standard mod-taps
// ---------------------------------------------------------------------------

bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t *tap_hold_record,
                      uint16_t other_keycode, keyrecord_t *other_record) {
  bool is_shift = false;

  if (IS_QK_MOD_TAP(tap_hold_keycode)) {
    uint8_t mod = QK_MOD_TAP_GET_MODS(tap_hold_keycode);
    is_shift = (mod == MOD_LSFT || mod == MOD_RSFT);

    // Prior idle: force tap if the mod-tap key was pressed too soon after
    // typing
    if (last_input_time != 0) {
      uint16_t idle_ms = is_shift ? PRIOR_IDLE_SHIFT : PRIOR_IDLE_OTHER;
      if (TIMER_DIFF_16(tap_hold_record->event.time, last_input_time) <
          idle_ms) {
        return false;
      }
    }
  }

  // Hand check
  char tap_hand =
      pgm_read_byte(&chordal_hold_layout[tap_hold_record->event.key.row]
                                        [tap_hold_record->event.key.col]);
  char other_hand =
      pgm_read_byte(&chordal_hold_layout[other_record->event.key.row]
                                        [other_record->event.key.col]);

  // Thumb keys: always allow hold
  if (tap_hand == '*' || other_hand == '*')
    return true;

  // Same hand: always tap (within 500ms tapping term)
  if (tap_hand == other_hand)
    return false;

  // Opposite hand: require minimum hold time before activating
  uint16_t hold_time =
      TIMER_DIFF_16(other_record->event.time, tap_hold_record->event.time);
  uint16_t required = is_shift ? HOLD_TIME_SHIFT : HOLD_TIME_OTHER;
  return hold_time >= required;
}

// ---------------------------------------------------------------------------

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  bool result = true;

  switch (keycode) {
  // Custom mod-taps (shifted keycodes that can't use standard MT)
  case ALT_LPRN:
    result =
        handle_custom_mt(keycode, record, KC_LALT, KC_LPRN, PRIOR_IDLE_OTHER);
    break;
  case CTL_RPRN:
    result =
        handle_custom_mt(keycode, record, KC_LCTL, KC_RPRN, PRIOR_IDLE_OTHER);
    break;
  case GUI_COLN:
    result = handle_custom_mt(keycode, record, KC_RGUI, S(KC_SCLN),
                              PRIOR_IDLE_OTHER);
    break;
  case CTL_LCBR:
    result = handle_custom_mt(keycode, record, KC_RCTL, S(KC_LBRC),
                              PRIOR_IDLE_OTHER);
    break;
  case ALT_RCBR:
    result = handle_custom_mt(keycode, record, KC_LALT, S(KC_RBRC),
                              PRIOR_IDLE_OTHER);
    break;
  // Russian letters on SYM_RU (OS is temporarily in EN, so flip back briefly)
  case RU_IO_KEY:
    if (record->event.pressed) {
      tap_code(KC_CAPS);
      tap_code(KC_GRV);
      tap_code(KC_CAPS);
    }
    result = false;
    break;
  case RU_HA_KEY:
    if (record->event.pressed) {
      tap_code(KC_CAPS);
      tap_code(KC_LBRC);
      tap_code(KC_CAPS);
    }
    result = false;
    break;
  case RU_HSG_KEY:
    if (record->event.pressed) {
      tap_code(KC_CAPS);
      tap_code(KC_RBRC);
      tap_code(KC_CAPS);
    }
    result = false;
    break;

  // Language toggle
  case LANG_SW:
    if (record->event.pressed) {
      if (is_russian) {
        layer_move(_EN);
        is_russian = false;
      } else {
        layer_move(_RU);
        is_russian = true;
      }
      tap_code(KC_CAPS);
    }
    result = false;
    break;
  }

  // Update last input time (for require-prior-idle tracking).
  if (record->event.pressed) {
    last_input_time = record->event.time;
  }

  return result;
}

// ---------------------------------------------------------------------------
// Layer state — auto-switch OS to English while SYM_RU is active
// ---------------------------------------------------------------------------

layer_state_t layer_state_set_user(layer_state_t state) {
  static bool sym_ru_was_active = false;
  bool sym_ru_is_active = layer_state_cmp(state, _SYM_RU);

  if (sym_ru_is_active && !sym_ru_was_active) {
    tap_code(KC_CAPS); // switch OS to English
  } else if (!sym_ru_is_active && sym_ru_was_active) {
    tap_code(KC_CAPS); // switch OS back to Russian
  }

  sym_ru_was_active = sym_ru_is_active;
  return state;
}

// ---------------------------------------------------------------------------
// Combos
//
// Under Vial, the combo engine is dynamic (stored in the device / set via the
// Entropy/Vial GUI), so this static table is compiled only when Vial's dynamic
// combos are NOT active. With the stock Phenom Micro Vial build these combos are
// therefore inactive — recreate them in Entropy if wanted, or use the physical
// reset for the bootloader.
// ---------------------------------------------------------------------------

#if !defined(VIAL_COMBO_ENABLE)
enum combo_names {
  COMBO_CAPS,   // CapsLock (all layers)
  COMBO_BOOT_L, // bootloader left  (top/home/bottom leftmost keys)
  COMBO_BOOT_R, // bootloader right (rightmost keys)
  COMBO_EECLR,  // clear EEPROM (outer thumbs: ` + Del)
  COMBO_LENGTH
};
uint16_t COMBO_LEN = COMBO_LENGTH;

// Combo key arrays (using EN base-layer keycodes since COMBO_ONLY_FROM_LAYER=0)
const uint16_t PROGMEM caps_combo[]   = {GUI_I, KC_SCLN, KC_DOT, COMBO_END};
const uint16_t PROGMEM boot_l_combo[] = {KC_F, GUI_S, KC_Z, COMBO_END};   // left column
const uint16_t PROGMEM boot_r_combo[] = {KC_COMM, GUI_I, KC_DOT, COMBO_END}; // right column
const uint16_t PROGMEM eeclr_combo[]  = {KC_GRV, KC_DEL, COMBO_END};       // outer thumbs

combo_t key_combos[] = {
    [COMBO_CAPS]   = COMBO(caps_combo, KC_CAPS),
    [COMBO_BOOT_L] = COMBO(boot_l_combo, QK_BOOT),
    [COMBO_BOOT_R] = COMBO(boot_r_combo, QK_BOOT),
    [COMBO_EECLR]  = COMBO(eeclr_combo, QK_CLEAR_EEPROM),
};
#endif // !VIAL_COMBO_ENABLE

// ---------------------------------------------------------------------------
// Keymap
//
// Phenom Micro LAYOUT() takes 42 args, in this order:
//   row0  (top)    : 10 keys
//   row1  (home)   : 10 keys
//   row2  (bottom) : 10 keys
//   thumbs         : 10 keys  (L outer->inner: 30..34, R inner->outer: 35..39)
//   encoders       : 2 keys   (left, right)
// ---------------------------------------------------------------------------

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /*
     * EN - English base layer
     *
     *      |  F |  D |  L |  B |  V |    |  J |  G |  O |  U |  , |
     *      |Gs  |At  |  R |Cn  |  K |    |  Y |Cm  |  A |Ae  |Gi  |
     *      |  Z |  Q |  X |  H |  P |    |  W |  C |  ' |  ; |  . |
     *   |`|ESC|SFT|NAV|SPC|              |ENT|SYM|BSP|TAB|DEL|
     */
    [_EN] = LAYOUT(
        KC_F,    KC_D,    KC_L,    KC_B,    KC_V,        KC_J,    KC_G,    KC_O,    KC_U,    KC_COMM,
        GUI_S,   ALT_T_,  KC_R,    CTL_N,   KC_K,        KC_Y,    CTL_M,   KC_A,    ALT_E,   GUI_I,
        KC_Z,    KC_Q,    KC_X,    KC_H,    KC_P,        KC_W,    KC_C,    KC_QUOT, KC_SCLN, KC_DOT,
        KC_ESC, KC_NO, KC_SPC, MO(_NAV), KC_LSFT,      KC_ENT, MO(_SYM_EN), KC_BSPC, KC_NO, KC_TAB,
        KC_MUTE, KC_MUTE),

    /*
     * RU - Russian base layer (OS must be in Russian mode)
     */
    [_RU] = LAYOUT(
        RU_J,    RU_TS,   RU_U,    RU_K,    RU_IE,       RU_N,    RU_G,    RU_SH,   RU_SCH,  RU_Z,
        GUI_RF,  ALT_RY,  RU_V,    CTL_RA,  RU_P,        RU_R,    CTL_RO,  RU_L,    ALT_RD,  GUI_RZH,
        RU_YA,   RU_CH,   RU_S,    RU_M,    RU_I,        RU_T,    RU_SS,   RU_B,    RU_YU,   RU_E,
        RU_HA, KC_ESC, KC_SPC, MO(_NAV), KC_LSFT,      KC_ENT, MO(_SYM_RU), KC_BSPC, KC_TAB, RU_IO,
        KC_MUTE, KC_MUTE),

    /*
     * SYM_EN - Symbols (English OS mode)
     *
     *      |  ~ |  < |  = |  > |  ! |    |  $ |  [ |  _ |  ] |  , |
     *      |G\  |A(  |  - |C)  |  + |    |  % |C{  |  ? |A}  |G:  |
     *      |  # |  * |  ` |  / |  & |    |  @ |  | |  " |  ; |  . |
     */
    [_SYM_EN] = LAYOUT(
        KC_TILD, KC_LT,    KC_EQL,  KC_GT,    KC_EXLM,    KC_DLR,  KC_LBRC,  KC_UNDS,    KC_RBRC,  KC_COMM,
        GUI_BSL, ALT_LPRN, KC_MINS, CTL_RPRN, KC_PLUS,    KC_PERC, CTL_LCBR, S(KC_SLSH), ALT_RCBR, GUI_COLN,
        KC_HASH, KC_ASTR,  KC_GRV,  KC_SLSH,  KC_AMPR,    KC_AT,   KC_PIPE,  KC_DQUO,    KC_SCLN,  KC_DOT,
        KC_TRNS, KC_TRNS, KC_TRNS, MO(_NAV), KC_LSFT,    KC_ENT, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS),

    /*
     * SYM_RU - Symbols (Russian OS mode) — same symbols as SYM_EN ([ _ ] etc.),
     * plus ъ on the outer-right thumb (OS is auto-flipped to EN while this layer
     * is active, so the RU_*_KEY keycodes briefly flip back to type Cyrillic).
     */
    [_SYM_RU] = LAYOUT(
        KC_TILD, KC_LT,    KC_EQL,  KC_GT,    KC_EXLM,    KC_DLR,  KC_LBRC,   KC_UNDS,    KC_RBRC,    KC_COMM,
        GUI_BSL, ALT_LPRN, KC_MINS, CTL_RPRN, KC_PLUS,    KC_PERC, CTL_LCBR,  S(KC_SLSH), ALT_RCBR,   GUI_COLN,
        KC_HASH, KC_ASTR,  KC_GRV,  KC_SLSH,  KC_AMPR,    KC_AT,   KC_PIPE,   KC_DQUO,    KC_SCLN,    KC_DOT,
        KC_TRNS, KC_TRNS, KC_TRNS, MO(_NAV), KC_LSFT,    KC_ENT, KC_NO, KC_TRNS, KC_TRNS, RU_HSG_KEY,
        KC_TRNS, KC_TRNS),

    /*
     * NAV - Navigation / Numbers
     *
     *      |  1 |  2 |  3 |  4 |  5 |    |  6 |  7 |  8 |  9 |  0 |
     *      |GUI |ALT |SFT |CTL |CSTb|    |COPY|C<- |dwn |A up|G ->|
     *      |WhUp|WhDn|RClk|LClk|CTab|    |PSTE|TAB |LANG|  ; |  . |
     */
    [_NAV] = LAYOUT(
        KC_1,    KC_2,    KC_3,    KC_4,    KC_5,             KC_6,            KC_7,    KC_8,    KC_9,   KC_0,
        KC_LGUI, KC_LALT, KC_LSFT, EH_SNP, C(S(KC_TAB)),      RCTL(RALT(KC_8)),CTL_LFT, KC_DOWN, ALT_UP, GUI_RGT,
        MS_WHLU, MS_WHLD, MS_BTN2, MS_BTN1, C(KC_TAB),        LALT(LCTL(KC_9)),KC_TAB,  LANG_SW, KC_SCLN,KC_DOT,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_LSFT,           S(KC_ENT), MO(_SYM_EN), KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS),
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_EN]     = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_RU]     = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_SYM_EN] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_SYM_RU] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_NAV]    = {ENCODER_CCW_CW(KC_WH_D, KC_WH_U), ENCODER_CCW_CW(KC_WH_D, KC_WH_U)},
};
#endif
