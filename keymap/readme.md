# Phenom Micro — `misha` keymap

Port of the TOTEM config (Russian/English dual-layer, home-row mods, custom
shifted mod-taps, CapsLock-based OS language switching) onto the Ergohaven
Phenom Micro (40 keys + trackball on the right half).

## Layers

0. `_EN`     — English base (home-row mods)
1. `_RU`     — Russian base (OS must be in Russian mode)
2. `_SYM_EN` — symbols (English OS)
3. `_SYM_RU` — symbols (Russian OS; auto-flips OS to EN while held)
4. `_NAV`    — navigation / numbers / mouse

Thumb cluster (Phenom Micro has 5 thumb keys per hand):

```
left  (outer→inner): Esc  --  Space  NAV  Shift   (RU outer = х)
right (inner→outer): Enter  SYM  Bspc  --  Tab     (RU outer = ё)
```

The trackball, OLED, RGB and split-pointing are handled by the Ergohaven
keyboard core and work unchanged.

## Differences vs the original TOTEM firmware

The Phenom Micro runs Ergohaven's Vial-QMK. Its settings system (needed for the
trackball/OLED/RGB) owns some behaviour, so a few things differ from the
hand-rolled TOTEM build:

- **Home-row mods**: our custom `get_chordal_hold()` (opposite-hand rule +
  require-prior-idle + 200 ms opposite-hand minimum) is preserved. To make this
  possible, `quantum/qmk_settings.c`'s `get_chordal_hold` was marked `__weak__`
  (one line; re-apply if you update the fork).
- **Combos** (CapsLock, bootloader, EEPROM-clear): under Vial the combo engine
  is dynamic, so the static C combos here are compiled out. Recreate them in
  Entropy/Vial if wanted, or use the physical reset to enter the bootloader.
- **Auto Shift**: force-enabled by the Vial build but runtime-OFF by default, so
  typing matches the TOTEM.
- **Auto-mouse layer**: disabled by default (the `auto_mouse_enable` default in
  `src/eh_pointing.c` is set to 0). The trackball only moves the cursor; reach
  NAV manually with the thumb like before. Mouse buttons and the sniper key live
  on NAV. (Requires clean settings — clear EEPROM via Bootmagic when flashing.)

## Trackball sensitivity

- Overall (normal mode) = sensor CPI: set to 832 via the trackball CPI table
  index 1 in `phenom_micro/rev1/pointing.c`.
- Sniper is custom-boosted FASTER than normal: sniper sens divisor = 1 in
  `src/eh_pointing.c`, and the SNIPER case in `pointing.c` multiplies motion by
  5/4 (with remainder accumulation) → 832 × 1.25 ≈ 1040. (Stock sniper can only
  be a fraction of CPI; this boost lets it exceed normal.) Scroll divisor (32)
  unchanged.
- These are stored settings, so clear EEPROM (Bootmagic: hold top-left while
  plugging in) when flashing, or set CPI/sniper-sens live in Entropy.

## Build

```sh
keyboards/ergohaven/phenom_micro/keymaps/misha/build.sh
```

Output: `~/qmk_totem_config/phenom_micro_misha.uf2`.

## Flash

1. Enter the bootloader on the half connected via USB:
   double-tap the reset button, or hold BOOT while plugging in.
   (Holding the **top-left key** while plugging in also clears stale settings.)
2. Copy `phenom_micro_misha.uf2` to the `RPI-RP2` drive that appears.
3. Repeat for the other half. Both halves need the same firmware.
