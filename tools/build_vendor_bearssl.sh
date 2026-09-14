#!/bin/sh
set -eu

CC=${CC:-cc}
AR=${AR:-ar}
CFLAGS=${CFLAGS:--Os -Wall -Wextra -std=c99}
VENDOR=${VENDOR:-build/vendor/BearSSL}
OUT=${OUT:-build/vendor-lib}

[ -d "$VENDOR/src" ] || { echo "missing vendor tree: $VENDOR" >&2; exit 1; }
[ -d "$VENDOR/inc" ] || { echo "missing vendor headers: $VENDOR/inc" >&2; exit 1; }
[ -f "$VENDOR/AMTLS-VENDOR-PROVENANCE.txt" ] || { echo "missing vendor provenance" >&2; exit 1; }

rm -rf "$OUT"
mkdir -p "$OUT/obj"

count=0
for src in $(find "$VENDOR/src" -type f -name '*.c' | sort); do
    rel=${src#"$VENDOR/"}
    obj="$OUT/obj/${rel%.c}.o"
    mkdir -p "$(dirname "$obj")"
    echo "BEARSSL_VENDOR_COMPILE=$rel"
    "$CC" $CFLAGS -I"$VENDOR/inc" -I"$VENDOR/src" -c "$src" -o "$obj"
    count=$((count + 1))
done

[ "$count" -gt 0 ] || { echo "vendor source set is empty" >&2; exit 1; }

find "$OUT/obj" -type f -name '*.o' | sort > "$OUT/objects.list"
# shellcheck disable=SC2046
"$AR" rcs "$OUT/libbearssl-amtls.a" $(cat "$OUT/objects.list")

cat > "$OUT/link_smoke.c" <<'EOF'
#include <bearssl.h>

int main(void)
{
    br_sha256_context hc;
    unsigned char out[32];
    br_sha256_init(&hc);
    br_sha256_update(&hc, "AmTLS", 5);
    br_sha256_out(&hc, out);
    return out[0] == 0xFF;
}
EOF

"$CC" $CFLAGS -I"$VENDOR/inc" "$OUT/link_smoke.c" "$OUT/libbearssl-amtls.a" -o "$OUT/link-smoke"

printf 'BEARSSL_VENDOR_OBJECTS=%s\n' "$count"
printf 'BEARSSL_VENDOR_LIBRARY=%s\n' "$OUT/libbearssl-amtls.a"
printf 'BEARSSL_VENDOR_LINK=PASS\n'
