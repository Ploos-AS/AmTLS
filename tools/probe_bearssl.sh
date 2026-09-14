#!/bin/sh
set -eu

REPO=${BEARSSL_REPO:-https://www.bearssl.org/git/BearSSL}
REF=${BEARSSL_REF:-refs/heads/master}
WORK=${1:-build/bearssl-probe}
CC=${CC:-cc}
CFLAGS=${CFLAGS:--O2 -Wall -Wextra -Werror -std=c99}

rm -rf "$WORK"
mkdir -p "$WORK"

git clone --quiet "$REPO" "$WORK/src"
cd "$WORK/src"
git checkout --quiet "$(git rev-parse "$REF")"

SHA=$(git rev-parse HEAD)
DATE=$(git show -s --format=%cI HEAD)
SUBJECT=$(git show -s --format=%s HEAD)
TREE=$(git rev-parse HEAD^{tree})
ARCHIVE_SHA256=$(git archive --format=tar "$SHA" | sha256sum | awk '{print $1}')

printf 'BEARSSL_REPOSITORY=%s\n' "$REPO"
printf 'BEARSSL_REF=%s\n' "$REF"
printf 'BEARSSL_COMMIT=%s\n' "$SHA"
printf 'BEARSSL_TREE=%s\n' "$TREE"
printf 'BEARSSL_COMMIT_DATE=%s\n' "$DATE"
printf 'BEARSSL_SUBJECT=%s\n' "$SUBJECT"
printf 'BEARSSL_GIT_ARCHIVE_SHA256=%s\n' "$ARCHIVE_SHA256"

compile_one() {
    f=$1
    printf 'BEARSSL_COMPILE_SOURCE=%s\n' "$f"
    if [ ! -f "$f" ]; then
        printf 'BEARSSL_COMPILE_MISSING=%s\n' "$f" >&2
        exit 1
    fi
    out="../$(basename "$f" .c).o"
    if ! "$CC" $CFLAGS -Iinc -Isrc -c "$f" -o "$out"; then
        printf 'BEARSSL_COMPILE_FAILED=%s\n' "$f" >&2
        exit 1
    fi
    printf 'BEARSSL_COMPILE_OK=%s\n' "$f"
}

# Representative files exercise the generic 15-bit integer, SHA-2,
# P-256, X.509 and TLS engine code paths without linking a target runtime.
# This is a compile qualification only, not a cryptographic runtime test.
compile_one src/int/i15_core.c
compile_one src/hash/sha2small.c
compile_one src/ec/ec_p256_i15.c
compile_one src/x509/x509_minimal.c
compile_one src/ssl/ssl_engine.c

printf 'BEARSSL_REPRESENTATIVE_COMPILE=PASS\n'
