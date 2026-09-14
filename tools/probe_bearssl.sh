#!/bin/sh
set -eu

REPO=${BEARSSL_REPO:-https://www.bearssl.org/git/BearSSL}
REF=${BEARSSL_REF:-refs/heads/master}
EXPECTED_SHA256=${BEARSSL_EXPECTED_SHA256:-}
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

if [ "$SHA" != "$REF" ] && printf '%s' "$REF" | grep -Eq '^[0-9a-f]{40}$'; then
    printf 'BEARSSL_COMMIT_MISMATCH expected=%s actual=%s\n' "$REF" "$SHA" >&2
    exit 1
fi
if [ -n "$EXPECTED_SHA256" ] && [ "$ARCHIVE_SHA256" != "$EXPECTED_SHA256" ]; then
    printf 'BEARSSL_ARCHIVE_SHA256_MISMATCH expected=%s actual=%s\n' "$EXPECTED_SHA256" "$ARCHIVE_SHA256" >&2
    exit 1
fi

find_source() {
    name=$1
    git ls-files | awk -v n="$name" '$0 == n || $0 ~ ("/" n "$") { print; exit }'
}

compile_component() {
    logical=$1
    f=$(find_source "$logical")
    if [ -z "$f" ]; then
        printf 'BEARSSL_COMPILE_MISSING=%s\n' "$logical" >&2
        exit 1
    fi

    case "$f" in
        src/*)
            root=.
            ;;
        */src/*)
            root=${f%%/src/*}
            ;;
        *)
            printf 'BEARSSL_COMPILE_UNEXPECTED_LAYOUT=%s\n' "$f" >&2
            exit 1
            ;;
    esac

    printf 'BEARSSL_COMPILE_SOURCE=%s\n' "$f"
    out="../$(basename "$f" .c).o"
    if ! "$CC" $CFLAGS -I"$root/inc" -I"$root/src" -c "$f" -o "$out"; then
        printf 'BEARSSL_COMPILE_FAILED=%s\n' "$f" >&2
        exit 1
    fi
    printf 'BEARSSL_COMPILE_OK=%s\n' "$f"
}

# Representative compile qualification for the pinned 2026 BearSSL tree.
compile_component int/i15_add.c
compile_component int/i15_montmul.c
compile_component int/i15_modpow.c
compile_component hash/sha2small.c
compile_component ec/ec_p256_m15.c
compile_component x509/x509_minimal.c
compile_component ssl/ssl_engine.c

printf 'BEARSSL_REPRESENTATIVE_COMPILE=PASS\n'
