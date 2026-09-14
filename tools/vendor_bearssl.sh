#!/bin/sh
set -eu

LOCK=${LOCK:-third_party/BearSSL.lock}
POLICY=${POLICY:-third_party/BearSSL.vendor}
DEST=${1:-build/vendor/BearSSL}
WORK=${2:-build/vendor-bearssl-src}

[ -f "$LOCK" ] || { echo "missing $LOCK" >&2; exit 1; }
[ -f "$POLICY" ] || { echo "missing $POLICY" >&2; exit 1; }

# shellcheck disable=SC1090
. "$LOCK"
[ "$status" = reviewed ] || { echo "BearSSL lock is not reviewed" >&2; exit 1; }

rm -rf "$WORK" "$DEST"
mkdir -p "$WORK" "$DEST"
git clone --quiet "$upstream" "$WORK/src"
cd "$WORK/src"
git checkout --quiet "$commit"

actual_commit=$(git rev-parse HEAD)
actual_archive=$(git archive --format=tar HEAD | sha256sum | awk '{print $1}')
[ "$actual_commit" = "$commit" ] || { echo "commit mismatch" >&2; exit 1; }
[ "$actual_archive" = "$archive_sha256" ] || { echo "archive SHA-256 mismatch" >&2; exit 1; }

repo_root=$(pwd)
policy_abs=$(cd ../../.. && pwd)/$POLICY
dest_abs=$(cd ../../.. && mkdir -p "$DEST" && cd "$DEST" && pwd)

matches_rule() {
    path=$1
    kind=$2
    while IFS='=' read -r key pattern; do
        [ "$key" = "$kind" ] || continue
        case "$path" in
            $pattern) return 0 ;;
        esac
    done < "$policy_abs"
    return 1
}

count=0
for path in $(git ls-files); do
    if matches_rule "$path" include && ! matches_rule "$path" exclude; then
        mkdir -p "$dest_abs/$(dirname "$path")"
        cp "$repo_root/$path" "$dest_abs/$path"
        count=$((count + 1))
    fi
done

license_path=
for candidate in LICENSE.txt LICENSE COPYING COPYING.txt; do
    if [ -f "$repo_root/$candidate" ]; then
        license_path=$candidate
        break
    fi
done
[ -n "$license_path" ] || { echo "upstream license file not found" >&2; exit 1; }
cp "$repo_root/$license_path" "$dest_abs/$license_path"

cat > "$dest_abs/AMTLS-VENDOR-PROVENANCE.txt" <<EOF
name=$name
upstream=$upstream
commit=$commit
archive_sha256=$archive_sha256
policy=$(basename "$POLICY")
files=$count
EOF

# Client-only policy invariants.
if find "$dest_abs/src/ssl" -type f \( -name 'ssl_server*' -o -name 'ssl_hs_server.c' -o -name 'ssl_scert_*' \) 2>/dev/null | grep -q .; then
    echo "server TLS source leaked into vendor tree" >&2
    exit 1
fi
if find "$dest_abs/src" -type f \( -name '*x86ni*' -o -name '*pwr8*' -o -name '*sse2*' \) 2>/dev/null | grep -q .; then
    echo "host-specific acceleration leaked into vendor tree" >&2
    exit 1
fi

printf 'BEARSSL_VENDOR_COMMIT=%s\n' "$commit"
printf 'BEARSSL_VENDOR_ARCHIVE_SHA256=%s\n' "$archive_sha256"
printf 'BEARSSL_VENDOR_FILES=%s\n' "$count"
printf 'BEARSSL_VENDOR_LICENSE=%s\n' "$license_path"
printf 'BEARSSL_VENDOR=PASS\n'
