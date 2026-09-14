#!/bin/sh
set -eu

CC=${CC:-cc}
AR=${AR:-ar}
CFLAGS=${CFLAGS:--Os -Wall -Wextra -Werror -std=c99}
VENDOR=${VENDOR:-build/vendor/BearSSL}
VENDOR_LIB=${VENDOR_LIB:-build/vendor-lib/libbearssl-amtls.a}
WORK=${WORK:-build/m2-1c-transport}
OUT=${OUT:-build/test-m2-1-transport-tls}

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

openssl x509 -in "$WORK/ca.pem" -outform DER -out "$WORK/ca.der"
openssl x509 -in "$WORK/other-ca.pem" -outform DER -out "$WORK/other-ca.der"

"$CC" $CFLAGS \
    -Iinclude -Isrc -I"$VENDOR/inc" \
    src/backends/handshake_pump.c \
    src/backends/bearssl_client.c \
    src/platform/allocator.c \
    src/backends/bearssl_portable_shim.c \
    tests/test_m2_1_transport_tls.c \
    "$VENDOR_LIB" \
    -o "$OUT"

run_case() {
    name=$1
    server_name=$2
    ca_file=$3
    expected=$4
    port=$(python3 - <<'PY'
import socket
s = socket.socket()
s.bind(("127.0.0.1", 0))
print(s.getsockname()[1])
s.close()
PY
)
    log="$WORK/${name}-server.log"
    # -www keeps the accepted TLS connection alive after the handshake and
    # waits for application data. Without it, s_server may observe EOF on its
    # inherited stdin in CI and close immediately after a successful handshake,
    # racing the AmTLS pump before it can observe BR_SSL_SENDAPP/RECVAPP.
    openssl s_server \
        -accept "127.0.0.1:$port" \
        -cert "$WORK/server.pem" \
        -key "$WORK/server.key" \
        -tls1_2 -quiet -www >"$log" 2>&1 &
    server_pid=$!
    trap 'kill "$server_pid" 2>/dev/null || true' EXIT HUP INT TERM
    sleep 1
    "$OUT" "$port" "$server_name" "$ca_file" "$expected"
    kill "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
    trap - EXIT HUP INT TERM
}

run_case trusted valid.example "$WORK/ca.der" ok
run_case hostname wrong.example "$WORK/ca.der" bad-name
run_case unknown valid.example "$WORK/other-ca.der" unknown-ca

printf '%s\n' 'AMTLS_TRANSPORT_TLS_TRUSTED=PASS'
printf '%s\n' 'AMTLS_TRANSPORT_TLS_HOSTNAME_MISMATCH=PASS'
printf '%s\n' 'AMTLS_TRANSPORT_TLS_UNKNOWN_CA=PASS'
printf '%s\n' 'AMTLS_M2_1C_TRANSPORT=PASS'
