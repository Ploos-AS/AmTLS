#!/bin/sh
set -eu

LOG="${1:-build/m3_3_fsuae/Test/T/amtls-m3.3.log}"

if test ! -s "$LOG"; then
    echo "M3.3 runtime qualification: FAIL (missing runtime log: $LOG)" >&2
    exit 1
fi

if grep -q '^AMTLS_M3_3_PASS' "$LOG" && grep -q '^RC=0' "$LOG"; then
    echo "M3.3 runtime qualification: PASS"
    exit 0
fi

echo "M3.3 runtime qualification: FAIL" >&2
cat "$LOG" >&2
exit 1
