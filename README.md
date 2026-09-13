# AmTLS

**Lightweight modern TLS for classic AmigaOS.**

AmTLS aims to provide a small, efficient TLS/HTTPS client layer for classic Amiga systems where the footprint and complexity of large TLS stacks are undesirable.

## M0 goals

M0 establishes the project baseline and architecture. It does **not** yet provide a production TLS implementation.

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
4. **Backend separation** — the public API must not expose backend-specific details. A compact TLS engine such as BearSSL is a candidate backend, but backend selection is intentionally deferred beyond M0.
5. **Socket-stack isolation** — network I/O is abstracted so the TLS core can integrate with Amiga TCP/IP stacks without coupling the public API to one stack.
6. **Secure defaults** — certificate and hostname validation are requirements before production use.
7. **Measurable efficiency** — later qualification will compare size, RAM use and performance against heavier alternatives where practical.

## Planned components

- `amtls.library` — Amiga-native TLS client API
- `TLSGet` — minimal HTTPS retrieval tool
- `TLSConnect` — TLS connection/test tool
- `TLSInfo` — connection, certificate and build information
- `TLSBench` — footprint/performance qualification utility

The CLI names above are roadmap targets; M0 contains only a portable host-side skeleton used to lock down interfaces and CI.

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

## Build M0

M0 is intentionally host-buildable so interface regressions can be caught without an Amiga runtime:

```sh
make
make check
```

The build uses only a C99 compiler at this stage.

## Status

**M0 — foundation / architecture baseline.** No cryptography is implemented yet and AmTLS must not be used for security-sensitive traffic until a later milestone is explicitly qualified.

## License

MIT. Copyright (c) Ploos AS.
