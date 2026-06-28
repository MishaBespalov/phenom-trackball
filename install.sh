#!/usr/bin/env bash
# Clone the Ergohaven vial-qmk fork, apply the core patch, link this keymap, and
# build. Re-runnable. Set VIAL_QMK to choose where the fork is cloned.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VIAL_QMK="${VIAL_QMK:-$HOME/trackball_firmware/vial-qmk}"

if [ ! -d "$VIAL_QMK/.git" ]; then
    echo "Cloning ergohaven/vial-qmk into $VIAL_QMK ..."
    git clone --depth 1 --branch ergohaven --recurse-submodules --shallow-submodules \
        https://github.com/ergohaven/vial-qmk.git "$VIAL_QMK"
fi

cd "$VIAL_QMK"

# Apply the core patch if not already applied.
if git apply --reverse --check "$HERE/patches/ergohaven-core.patch" 2>/dev/null; then
    echo "Core patch already applied."
else
    echo "Applying core patch ..."
    git apply "$HERE/patches/ergohaven-core.patch"
fi

# Link (or copy) the keymap into the fork.
KM_DIR="$VIAL_QMK/keyboards/ergohaven/phenom_micro/keymaps/misha"
rm -rf "$KM_DIR"
cp -r "$HERE/keymap" "$KM_DIR"

# Build via the keymap's build helper (handles NixOS toolchain + qmk_home).
exec "$KM_DIR/build.sh"
