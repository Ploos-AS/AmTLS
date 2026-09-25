#!/bin/sh
set -eu

LOG_A="${1:-build/m3_3_fsuae/Test/T/amtls-m3.3.log}"
LOG_B="${2:-${LOG_A%/*}/amtls-m3.3b.log}"

fail=0
for log in "$LOG_A" "$LOG_B"; do
    if test ! -s "$log"; then
        echo "M3.3 runtime qualification: FAIL (missing runtime log: $log)" >&2
        fail=1
    fi
done
test "$fail" -eq 0 || exit 1

if grep -qx 'RC=0' "$LOG_A" &&
   grep -qx 'AMTLS_M3_3_PASS' "$LOG_A" &&
   grep -qx 'RC=0' "$LOG_B" &&
   grep -qx 'AMTLS_M3_3B_PASS' "$LOG_B"; then
    echo "M3.3a/b runtime qualification: PASS"
    exit 0
fi

echo "M3.3a/b runtime qualification: FAIL" >&2
echo "--- $LOG_A ---" >&2
cat "$LOG_A" >&2
echo "--- $LOG_B ---" >&2
cat "$LOG_B" >&2
exit 1
