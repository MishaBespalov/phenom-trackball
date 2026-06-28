#!/usr/bin/env bash
# Build the Phenom Micro "misha" firmware.
# Handles the NixOS ARM toolchain + pointing qmk at this vial-qmk checkout.
set -euo pipefail

ARM_BIN="/nix/store/c353gqsmf8mvg72vivm9fb2dv210wnkm-gcc-arm-embedded-14.2.rel1/bin"
VIAL_QMK="$HOME/trackball_firmware/vial-qmk"

export PATH="$ARM_BIN:$PATH"

# qmk uses its configured home (not env), so set/restore it around the build.
PREV_HOME="$(qmk config user.qmk_home | cut -d= -f2 || true)"
qmk config "user.qmk_home=$VIAL_QMK" >/dev/null
trap 'qmk config "user.qmk_home=$PREV_HOME" >/dev/null' EXIT

cd "$VIAL_QMK"
qmk compile -kb ergohaven/phenom_micro/rev1 -km misha

cp .build/ergohaven_phenom_micro_rev1_misha.uf2 "$HOME/qmk_totem_config/phenom_micro_misha.uf2"
echo "Built: $HOME/qmk_totem_config/phenom_micro_misha.uf2"
