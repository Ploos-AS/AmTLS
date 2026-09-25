#!/bin/sh
set -eu

# Run the staged M3.3 payload with FS-UAE.
#
# Required, user-supplied inputs:
#   AMTLS_KICKSTART_ROM=/path/to/legal/kickstart.rom
#   AMTLS_SYSTEM_DIR=/path/to/legal/bootable AmigaOS directory
#
# Neither input belongs in this repository.

FS_UAE="${FS_UAE:-fs-uae}"
PAYLOAD_DIR="${PAYLOAD_DIR:-build/m3_3_runtime}"
RUN_DIR="${RUN_DIR:-build/m3_3_fsuae}"
TIMEOUT="${AMTLS_FS_UAE_TIMEOUT:-120}"

: "${AMTLS_KICKSTART_ROM:?set AMTLS_KICKSTART_ROM to a legal Kickstart ROM}"
: "${AMTLS_SYSTEM_DIR:?set AMTLS_SYSTEM_DIR to a legal bootable AmigaOS directory}"

test -f "$AMTLS_KICKSTART_ROM"
test -d "$AMTLS_SYSTEM_DIR"
test -s "$PAYLOAD_DIR/Libs/amtls.library"
test -s "$PAYLOAD_DIR/C/test_m3_3_openclose"
command -v "$FS_UAE" >/dev/null 2>&1

rm -rf "$RUN_DIR"
mkdir -p "$RUN_DIR/Test/Libs" "$RUN_DIR/Test/C" "$RUN_DIR/Test/S" "$RUN_DIR/Test/T"
cp "$PAYLOAD_DIR/Libs/amtls.library" "$RUN_DIR/Test/Libs/"
cp "$PAYLOAD_DIR/C/test_m3_3_openclose" "$RUN_DIR/Test/C/"

cat > "$RUN_DIR/Test/S/run-test" <<'EOF'
Assign LIBS: TEST:Libs ADD
Assign C: TEST:C ADD
C:test_m3_3_openclose >TEST:T/amtls-m3.3.log
Echo "RC=$RC" >>TEST:T/amtls-m3.3.log
If $RC EQ 0
  Echo "AMTLS_M3_3_PASS" >>TEST:T/amtls-m3.3.log
Else
  Echo "AMTLS_M3_3_FAIL" >>TEST:T/amtls-m3.3.log
EndIf
EOF

cat > "$RUN_DIR/m3_3.fs-uae" <<EOF
[fs-uae]
amiga_model = A500+
kickstart_file = $AMTLS_KICKSTART_ROM
hard_drive_0 = $AMTLS_SYSTEM_DIR
hard_drive_0_label = DH0
hard_drive_1 = $RUN_DIR/Test
hard_drive_1_label = TEST
accuracy = -1
uae_cpu_speed = max
fullscreen = 0
EOF

echo "FS-UAE payload prepared."
echo "Boot the configured AmigaOS system and run:"
echo "  Execute TEST:S/run-test"
echo
echo "For automated environments, arrange the supplied system image startup"
echo "to execute TEST:S/run-test and quit the emulator afterwards."
echo "Then verify with:"
echo "  tools/verify_m3_3_runtime.sh $RUN_DIR/Test/T/amtls-m3.3.log"
echo
echo "Launching FS-UAE (timeout: ${TIMEOUT}s)..."
timeout "$TIMEOUT" "$FS_UAE" "$RUN_DIR/m3_3.fs-uae" || rc=$?
rc=${rc:-0}
case "$rc" in
  0|124) ;;
  *) echo "FS-UAE exited with status $rc" >&2; exit "$rc" ;;
esac

tools/verify_m3_3_runtime.sh "$RUN_DIR/Test/T/amtls-m3.3.log"
