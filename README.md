# Phenom Micro — Misha's config

Custom QMK config for the **Ergohaven Phenom Micro** (split, trackball on the
right half), ported from a TOTEM keymap. Russian/English dual-layer with
home-row mods, custom chordal hold, custom shifted mod-taps, CapsLock-based OS
language switching, and trackball tuning (sniper, sensitivity, no auto-mouse).

The Phenom Micro runs Ergohaven's **Vial-QMK** fork. This repo holds the parts
that are *mine* on top of that fork:

```
keymap/                    the keymap (-> keyboards/ergohaven/phenom_micro/keymaps/misha/)
  keymap.c                 layers, combos, custom keycodes, chordal hold, language switch
  config.h                 timings, Vial UID, mouse-wheel tuning
  rules.mk                 Vial enable
  vial.json                Vial layout (copied from the stock v1 keymap)
  build.sh                 build helper (NixOS toolchain + qmk paths)
  readme.md               keymap-level notes
patches/
  ergohaven-core.patch     3 small edits to fork core files (see below)
  BASE.txt                 the fork commit these were based on
firmware/
  phenom_micro_misha.uf2   prebuilt firmware (flash to BOTH halves)
install.sh                 clone fork + apply patch + link keymap + build
```

## Core-file tweaks (`patches/ergohaven-core.patch`)

The Vial settings system owns some behaviour, so three fork files are patched:

1. `quantum/qmk_settings.c` — `get_chordal_hold` made `__weak__` so the keymap's
   custom `get_chordal_hold` (opposite-hand + require-prior-idle) takes over.
2. `keyboards/ergohaven/src/eh_pointing.c` — auto-mouse default OFF; sniper sens
   divisor = 1 (raw, then boosted in motion code).
3. `keyboards/ergohaven/phenom_micro/rev1/pointing.c` — trackball CPI 832
   (overall), and the SNIPER motion path boosted ×5/4 so sniper can be *faster*
   than normal pointing.

## Build

Needs the ARM toolchain + `qmk` CLI. On the original machine:

```sh
./install.sh          # clones the fork, applies the patch, links the keymap, builds
```

Or manually:

```sh
git clone --depth 1 --branch ergohaven --recurse-submodules --shallow-submodules \
    https://github.com/ergohaven/vial-qmk.git
cd vial-qmk
git apply ../patches/ergohaven-core.patch
cp -r ../keymap keyboards/ergohaven/phenom_micro/keymaps/misha
qmk config user.qmk_home="$PWD"
qmk compile -kb ergohaven/phenom_micro/rev1 -km misha
# -> .build/ergohaven_phenom_micro_rev1_misha.uf2
```

## Flash (both halves)

1. Enter the bootloader on the half plugged into USB. **Hold the top-left key
   while plugging in** (Bootmagic) — this also clears stale settings so the
   trackball/auto-mouse defaults apply. (Plain double-tap reset works too, but
   won't clear settings.)
2. Copy `phenom_micro_misha.uf2` to the `RPI-RP2` drive that appears.
3. Repeat for the other half. Both halves need the same firmware.

## Config summary

- **Layers:** `_EN` `_RU` `_SYM_EN` `_SYM_RU` `_NAV`
- **Home-row mods** with custom chordal hold (opposite-hand, require-prior-idle).
- **Shifted mod-taps** (e.g. hold Alt / tap `(`), language switch via CapsLock.
- **Thumbs (EN):** Esc on outer-left, Tab on outer-right; RU outer thumbs = х / ё.
- **NAV:** numbers/nav/mouse; sniper (`EH_SNP`) on the left index home key
  (tap-toggle + hold-momentary).
- **Trackball:** overall CPI 832; sniper ~1040 (boosted, faster than normal);
  auto-mouse disabled (reach NAV with the thumb).

## OS requirements

US + Russian layouts with CapsLock as the language toggle:

```
kb_layout = us,ru
kb_options = grp:caps_toggle
```
