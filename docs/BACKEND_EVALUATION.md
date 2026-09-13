# TLS backend evaluation

## M1 decision

BearSSL is the first backend candidate for AmTLS because its design is compact, embedded-oriented and does not require exposing an OpenSSL-compatible API to applications.

M1 deliberately keeps the AmTLS public API and transport layer independent of BearSSL. This lets the project replace or supplement the backend later without breaking Amiga applications.

## Selection criteria

A backend must be evaluated on:

- 68000 code-generation compatibility
- code size
- resident and peak RAM use
- TLS 1.2 client capability
- certificate-chain and hostname verification support
- entropy requirements
- absence of mandatory threads/FPU
- maintainability and licensing
- performance on 68000/020/030/060 targets

## Pinning policy

Do not track an unreviewed moving branch in production builds. Before the first functional TLS milestone, AmTLS will vendor or otherwise pin a reviewed BearSSL source revision and record its origin, commit/release identifier and integrity hash.

## Current status

Architecture: accepted as first candidate.
Source revision: not yet pinned.
Production use: not yet permitted.

This distinction is intentional: M1 locks the backend boundary, not an unreviewed third-party source snapshot.
