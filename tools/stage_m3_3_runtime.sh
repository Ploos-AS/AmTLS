#!/bin/sh
set -eu

# M3.3 runtime payload staging.
#
# This script deliberately contains no Kickstart ROM, Workbench, AmigaOS,
# or other proprietary files.  It stages only AmTLS-built artifacts plus
# an AmigaDOS startup script.  A local/CI runtime harness may mount the
# resulting directory as a writable/read-only filesystem in FS-UAE.

BUILD_DIR="${BUILD_DIR:-build/amigaos}"
OUT_DIR="${OUT_DIR:-build/m3_3_runtime}"

LIB="$BUILD_DIR/amtls.library"
TEST="$BUILD_DIR/test_m3_3_openclose"
BSDTEST="$BUILD_DIR/test_m3_3_bsdsocket"

test -s "$LIB"
test -s "$TEST"
test -s "$BSDTEST"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/Libs" "$OUT_DIR/C" "$OUT_DIR/S" "$OUT_DIR/T"

test -s runtime/amiga-runtime.json
cp runtime/amiga-runtime.json "$OUT_DIR/amiga-runtime.json"

cp "$LIB" "$OUT_DIR/Libs/amtls.library"
cp "$TEST" "$OUT_DIR/C/test_m3_3_openclose"
cp "$BSDTEST" "$OUT_DIR/C/test_m3_3_bsdsocket"

cat > "$OUT_DIR/S/Startup-Sequence" <<'EOF'
FailAt 21
C:test_m3_3_openclose >T:amtls-m3.3.log
SetEnv AmTLSRC $RC
Echo "RC=$RC" >>T:amtls-m3.3.log
If $RC EQ 0
  Echo "AMTLS_M3_3_PASS" >>T:amtls-m3.3.log
Else
  Echo "AMTLS_M3_3_FAIL" >>T:amtls-m3.3.log
EndIf

C:test_m3_3_bsdsocket >T:amtls-m3.3b.log
Echo "RC=$RC" >>T:amtls-m3.3b.log
If $RC EQ 0
  Echo "AMTLS_M3_3B_PASS" >>T:amtls-m3.3b.log
Else
  Echo "AMTLS_M3_3B_FAIL" >>T:amtls-m3.3b.log
EndIf
EOF

cat > "$OUT_DIR/README.txt" <<'EOF'
AmTLS M3.3 runtime payload

Mount this directory in a legal AmigaOS runtime with its Libs/ directory
available as LIBS: and C/ available as C:, or copy the files into a test
volume before executing S/Startup-Sequence.

Expected success markers:
  AMTLS_M3_3_PASS
  AMTLS_M3_3B_PASS

The payload intentionally contains no Kickstart ROM or AmigaOS files.
EOF

echo "Staged M3.3 runtime payload in $OUT_DIR"
