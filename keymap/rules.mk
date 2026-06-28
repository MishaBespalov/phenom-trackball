# Vial is required: the Ergohaven core (eh_settings / QMK Settings, used by the
# trackball, OLED and RGB) only builds with the Vial settings system enabled.
VIAL_ENABLE = yes

# Auto Shift is force-enabled by the Vial settings build, but it is runtime-OFF
# by default (controlled via QMK Settings), so typing behaves like the TOTEM.

# COMBO / MOUSEKEY / POINTING(trackball) / RGB / OLED are enabled at the
# keyboard level and are inherited as-is.
