#!/bin/sh
set -eu

CC=${CC:-cc}
AR=${AR:-ar}
CFLAGS=${CFLAGS:--Os -Wall -Wextra -Werror -std=c99}
VENDOR=${VENDOR:-build/vendor/BearSSL}
VENDOR_LIB=${VENDOR_LIB:-build/vendor-lib/libbearssl-amtls.a}
WORK=${WORK:-build/m2-1c-x509}
OUT=${OUT:-build/test-m2-1-x509}

rm -rf "$WORK"
mkdir -p "$WORK"

chmod +x tools/vendor_bearssl.sh tools/build_vendor_bearssl.sh
tools/vendor_bearssl.sh "$VENDOR"
CC="$CC" AR="$AR" CFLAGS="$CFLAGS" VENDOR="$VENDOR" \
    tools/build_vendor_bearssl.sh

openssl genrsa -out "$WORK/ca.key" 2048 >/dev/null 2>&1
openssl req -x509 -new -sha256 -days 3650 \
    -key "$WORK/ca.key" \
    -subj '/CN=AmTLS Test CA' \
    -out "$WORK/ca.pem"

openssl genrsa -out "$WORK/other-ca.key" 2048 >/dev/null 2>&1
openssl req -x509 -new -sha256 -days 3650 \
    -key "$WORK/other-ca.key" \
    -subj '/CN=AmTLS Other CA' \
    -out "$WORK/other-ca.pem"

openssl genrsa -out "$WORK/server.key" 2048 >/dev/null 2>&1
openssl req -new \
    -key "$WORK/server.key" \
    -subj '/CN=valid.example' \
    -out "$WORK/server.csr"
cat > "$WORK/server.ext" <<'EOF'
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=DNS:valid.example
EOF
openssl x509 -req -sha256 -days 365 \
    -in "$WORK/server.csr" \
    -CA "$WORK/ca.pem" -CAkey "$WORK/ca.key" -CAcreateserial \
    -extfile "$WORK/server.ext" \
    -out "$WORK/server.pem" >/dev/null 2>&1

openssl x509 -in "$WORK/server.pem" -outform DER -out "$WORK/server.der"
openssl x509 -in "$WORK/ca.pem" -outform DER -out "$WORK/ca.der"
openssl x509 -in "$WORK/other-ca.pem" -outform DER -out "$WORK/other-ca.der"

"$CC" $CFLAGS \
    -Iinclude -Isrc -I"$VENDOR/inc" \
    src/backends/handshake_pump.c \
    src/backends/bearssl_client.c \
    src/platform/allocator.c \
    src/backends/bearssl_portable.c \
    tests/test_m2_1_x509.c \
    "$VENDOR_LIB" \
    -o "$OUT"

"$OUT" "$WORK/server.der" "$WORK/ca.der" "$WORK/other-ca.der"
printf '%s\n' 'AMTLS_X509_TRUSTED_HOST=PASS'
printf '%s\n' 'AMTLS_X509_HOSTNAME_MISMATCH=PASS'
printf '%s\n' 'AMTLS_X509_UNKNOWN_CA=PASS'
printf '%s\n' 'AMTLS_M2_1C_X509=PASS'
