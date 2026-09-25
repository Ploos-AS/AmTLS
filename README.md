# AmTLS

**Lightweight modern TLS for classic AmigaOS.**

AmTLS aims to provide a small, efficient TLS/HTTPS client layer for classic Amiga systems where the footprint and complexity of large TLS stacks are undesirable.

## Current goals

AmTLS has progressed through the portable TLS client, verification, bsdsocket transport, and native AmigaOS library build milestones. The current milestone is **M3.3 runtime qualification**.

Primary targets:

- AmigaOS 2.04+
- Motorola 68000 baseline
- C API suitable for `amtls.library`
- small command-line utilities for diagnostics and qualification
- cross-compilation as the normal development workflow
- TLS client use-cases first; server support is out of scope for the initial milestones

## Design principles

1. **Small footprint first** — code size, resident memory and peak handshake memory are first-class metrics.
2. **Classic hardware first** — the baseline must remain usable on 68000-class systems.
3. **Stable Amiga-native API** — applications should not need an OpenSSL-compatible API.
4. **Backend separation** — the public API must not expose backend-specific details. The current qualified backend is a pinned and reviewed BearSSL revision; backend details remain isolated from the public API.
5. **Socket-stack isolation** — network I/O is abstracted so the TLS core can integrate with Amiga TCP/IP stacks without coupling the public API to one stack.
6. **Secure defaults** — certificate and hostname validation are requirements before production use.
7. **Measurable efficiency** — later qualification will compare size, RAM use and performance against heavier alternatives where practical.

## Planned components

- `amtls.library` — Amiga-native TLS client API
- `TLSGet` — minimal HTTPS retrieval tool
- `TLSConnect` — TLS connection/test tool
- `TLSInfo` — connection, certificate and build information
- `TLSBench` — footprint/performance qualification utility

The CLI suite remains a roadmap target. `TLSInfo` currently exists as a build/diagnostic skeleton; the remaining CLI functionality follows runtime qualification.

## Repository layout

```text
include/amtls/       public API headers
src/                 portable core scaffolding
src/backends/        TLS backend boundary
src/platform/        platform/socket boundary
cli/                 command-line scaffolding
tests/               host-side contract tests
docs/                architecture and roadmap
```

## Build and test

The portable core remains host-buildable so interface regressions can be caught without an Amiga runtime:

```sh
make
make check
```

Native AmigaOS/68000 artifacts are also built in CI with `m68k-amigaos-gcc`.

## Status

**M3.2 — PASS. M3.3 runtime qualification is next.**\n\nCompleted qualification includes the portable core, pinned BearSSL backend, TLS 1.2 ClientHello/SNI path, trust/X.509 and hostname-negative tests, bsdsocket-compatible transport, and native AmigaOS/68000 `amtls.library` build/lifecycle qualification.\n\nAmTLS is **not yet a production release**. Real AmigaOS runtime qualification remains a release gate.

## License

MIT. Copyright (c) Ploos AS.
