#!/bin/sh
set -eu

LOCK=${BEARSSL_LOCK:-third_party/BearSSL.lock}
WORK=${1:-build/bearssl-host-qual}

get_lock() {
    key=$1
    sed -n "s/^${key}=//p" "$LOCK" | head -1
}

REPO=$(get_lock upstream)
SHA=$(get_lock commit)
EXPECTED_ARCHIVE=$(get_lock archive_sha256)
STATUS=$(get_lock status)

if [ "$STATUS" != "reviewed" ]; then
    echo "BEARSSL_HOST_QUAL=BLOCKED status=$STATUS" >&2
    exit 1
fi

rm -rf "$WORK"
mkdir -p "$WORK"
git clone --quiet "$REPO" "$WORK/src"
cd "$WORK/src"
git checkout --quiet "$SHA"

ACTUAL_SHA=$(git rev-parse HEAD)
ACTUAL_ARCHIVE=$(git archive --format=tar "$ACTUAL_SHA" | sha256sum | awk '{print $1}')

if [ "$ACTUAL_SHA" != "$SHA" ]; then
    echo "BEARSSL_HOST_QUAL_SHA_MISMATCH expected=$SHA actual=$ACTUAL_SHA" >&2
    exit 1
fi
if [ "$ACTUAL_ARCHIVE" != "$EXPECTED_ARCHIVE" ]; then
    echo "BEARSSL_HOST_QUAL_ARCHIVE_MISMATCH expected=$EXPECTED_ARCHIVE actual=$ACTUAL_ARCHIVE" >&2
    exit 1
fi

printf 'BEARSSL_HOST_COMMIT=%s\n' "$ACTUAL_SHA"
printf 'BEARSSL_HOST_ARCHIVE_SHA256=%s\n' "$ACTUAL_ARCHIVE"

make -j2

if [ ! -x build/testcrypto ]; then
    echo "BEARSSL_HOST_QUAL_MISSING=build/testcrypto" >&2
    exit 1
fi
if [ ! -x build/testx509 ]; then
    echo "BEARSSL_HOST_QUAL_MISSING=build/testx509" >&2
    exit 1
fi

./build/testcrypto all
./build/testx509

printf '%s\n' 'BEARSSL_HOST_CRYPTO=PASS'
printf '%s\n' 'BEARSSL_HOST_X509=PASS'
printf '%s\n' 'BEARSSL_HOST_QUALIFICATION=PASS'
