# TLS backend provenance

AmTLS treats the TLS backend as security-critical third-party code. A moving branch or an unverified archive is never accepted as a production dependency.

## BearSSL candidate

BearSSL remains the first backend candidate because its compact design and TLS 1.2/X.509 implementation fit the classic-Amiga goals. The canonical upstream is `https://www.bearssl.org/git/BearSSL`.

The numbered BearSSL release v0.6 dates from 2018. Upstream development continued after that release and the canonical repository shows later fixes, including commits in April 2026. AmTLS therefore does **not** pin v0.6 merely because it is the most recent numbered release.

## M2.0 gate

`third_party/BearSSL.lock` is the single source of truth for backend provenance.

Before functional TLS integration it must transition from:

`status=blocked-pending-review`

to:

`status=reviewed`

A reviewed lock requires:

- canonical upstream URL
- full immutable 40-hex Git commit ID
- SHA-256 of the exact reviewed source archive/snapshot
- license declaration
- review notes identifying relevant post-v0.6 fixes and any Amiga-specific patches

`tools/verify_backend_lock.py` checks these invariants in CI. The blocked state is deliberately valid for the repository, but means no BearSSL code may be linked into AmTLS yet.

## Review checklist

1. Resolve the canonical upstream master commit intended for AmTLS.
2. Inspect all changes since v0.6 with special attention to TLS record parsing, CBC handling, X.509 validation, RSA/ECC arithmetic and entropy-related code.
3. Archive the selected immutable revision and record its SHA-256.
4. Confirm MIT license text is retained with vendored source.
5. Compile the selected subset with the 68000 cross-compiler using conservative feature selection.
6. Run upstream crypto/X.509 tests on a host build before enabling AmTLS handshake integration.
7. Document any local patch as a separate patch file with rationale; never silently modify vendored cryptographic code.

## Policy

Dependency updates are explicit security changes. They require a new immutable commit/hash pair and requalification; AmTLS will not automatically follow BearSSL `master`.
