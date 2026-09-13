#!/usr/bin/env python3
"""Validate the AmTLS TLS-backend provenance lock.

M2.0 distinguishes two states:
- blocked-pending-review: valid repository state, backend integration disabled;
- reviewed: immutable 40-hex commit and 64-hex archive SHA-256 are mandatory.
"""

from pathlib import Path
import re
import sys

LOCK = Path("third_party/BearSSL.lock")
REQUIRED = {
    "name",
    "upstream",
    "candidate",
    "commit",
    "archive_sha256",
    "license",
    "status",
}


def load_lock(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise ValueError(f"invalid lock line: {raw!r}")
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()
    return values


def main() -> int:
    if not LOCK.is_file():
        print(f"ERROR: missing {LOCK}", file=sys.stderr)
        return 1

    try:
        values = load_lock(LOCK)
    except (OSError, ValueError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    missing = sorted(REQUIRED - values.keys())
    if missing:
        print("ERROR: missing lock keys: " + ", ".join(missing), file=sys.stderr)
        return 1

    if values["name"] != "BearSSL":
        print("ERROR: unexpected backend name", file=sys.stderr)
        return 1
    if values["upstream"] != "https://www.bearssl.org/git/BearSSL":
        print("ERROR: BearSSL upstream must be the canonical repository", file=sys.stderr)
        return 1
    if values["license"] != "MIT":
        print("ERROR: unexpected backend license declaration", file=sys.stderr)
        return 1

    status = values["status"]
    if status == "blocked-pending-review":
        if values["commit"] != "UNREVIEWED" or values["archive_sha256"] != "UNREVIEWED":
            print("ERROR: blocked lock must not contain partially reviewed provenance", file=sys.stderr)
            return 1
        print("BearSSL lock: valid, integration blocked pending source review")
        return 0

    if status != "reviewed":
        print(f"ERROR: unsupported backend status: {status}", file=sys.stderr)
        return 1

    if not re.fullmatch(r"[0-9a-f]{40}", values["commit"]):
        print("ERROR: reviewed backend requires full 40-hex commit", file=sys.stderr)
        return 1
    if not re.fullmatch(r"[0-9a-f]{64}", values["archive_sha256"]):
        print("ERROR: reviewed backend requires 64-hex archive SHA-256", file=sys.stderr)
        return 1

    print(f"BearSSL lock: reviewed commit {values['commit']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
