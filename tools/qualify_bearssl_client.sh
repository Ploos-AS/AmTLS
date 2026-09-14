#!/bin/sh
set -eu

CC=${CC:-cc}
AR=${AR:-ar}
CFLAGS=${CFLAGS:--Os -Wall -Wextra -Werror -std=c99}
RUN_TEST=${RUN_TEST:-1}
VENDOR=${VENDOR:-build/vendor/BearSSL}
VENDOR_LIB=${VENDOR_LIB:-build/vendor-lib/libbearssl-amtls.a}
OUT=${OUT:-build/test-m2-1-bearssl}

chmod +x tools/vendor_bearssl.sh tools/build_vendor_bearssl.sh
tools/vendor_bearssl.sh "$VENDOR"
CC="$CC" AR="$AR" CFLAGS="$CFLAGS" VENDOR="$VENDOR" \
    tools/build_vendor_bearssl.sh

"$CC" $CFLAGS \
    -Iinclude -Isrc -I"$VENDOR/inc" \
    src/platform/allocator.c \
    src/backends/handshake_pump.c \
    src/backends/bearssl_client.c \
    tests/test_m2_1_bearssl.c \
    "$VENDOR_LIB" \
    -o "$OUT"

if [ "$RUN_TEST" = 1 ]; then
    "$OUT"
fi

printf 'AMTLS_BEARSSL_CLIENT_LINK=PASS\n'
if [ "$RUN_TEST" = 1 ]; then
    printf 'AMTLS_BEARSSL_CLIENTHELLO=PASS\n'
fi
