# AmTLS roadmap

## M0 — Foundation

- [x] define AmigaOS 2.04+/68000 baseline
- [x] define lightweight-first design goals
- [x] establish backend-independent public C API skeleton
- [x] establish host-buildable core and TLSInfo skeleton
- [x] add host-side contract test
- [x] document architecture and security qualification requirements
- [x] add CI baseline

## M1 — Backend and transport foundation

- [x] define internal TLS backend interface
- [x] evaluate compact TLS backend architecture (BearSSL first candidate)
- [x] define transport callback API
- [x] implement deterministic host test transport
- [x] establish memory/allocation instrumentation
- [x] add 68000 cross-compile CI smoke build
- [x] fix 68000 CI smoke build directory dependency

## M2 — TLS client handshake

### M2.0 — Backend provenance gate

- [x] establish canonical-backend provenance policy
- [x] add machine-checked backend lock format
- [x] explicitly reject moving/unreviewed refs for functional TLS integration
- [x] record why BearSSL v0.6 is not automatically acceptable (upstream fixes continued after release)
- [x] resolve and review exact upstream BearSSL commit used by AmTLS
- [x] pin immutable 40-hex commit and archive SHA-256
- [x] define minimal client-only vendor policy
- [x] add reproducible vendor-tree generator with provenance and upstream license preservation
- [ ] commit reviewed source subset under third_party/vendor/BearSSL
- [x] host crypto/X.509 qualification of pinned backend
- [x] 68000 compile qualification of selected backend subset
- [x] enforce pinned commit and archive SHA-256 in CI
- [x] compile and link generated minimal vendor subset as a standalone library on host and 68000

### M2.1 — Functional TLS client

#### M2.1a — Transport-driven handshake pump

- [x] define backend-independent TLS engine record states
- [x] pump outbound TLS records through transport callbacks with partial-write handling
- [x] pump inbound TLS records through transport callbacks with partial-read handling
- [x] distinguish WANT_READ, WANT_WRITE, CLOSED and ERROR outcomes
- [x] deterministic host tests for partial I/O, would-block, EOF and engine error
- [x] include handshake-pump source in 68000 smoke compile

#### M2.1b — BearSSL client binding

- [x] bind BearSSL engine record buffers to handshake pump
- [x] TLS client state machine
- [x] SNI/server-name handling
- [x] TLS 1.2-only protocol policy
- [x] entropy injection contract that fails closed
- [x] host ClientHello/SNI qualification
- [x] 68000 compile/link qualification

#### M2.1c — Trust and verification

- [x] define CA trust loading strategy suitable for classic Amiga
- [x] require explicit non-empty trust-anchor set before handshake
- [x] wire trust anchors into BearSSL minimal X.509 validator
- [x] hostname verification contract via required server name
- [x] negative test: missing trust fails closed before ClientHello
- [x] end-to-end certificate-chain validation fixture through AmTLS transport
- [x] end-to-end hostname mismatch fixture through AmTLS transport
- [x] end-to-end unknown-CA fixture through AmTLS transport

## M3 — Amiga network integration

### M3.1 — bsdsocket-compatible transport adapter

- [x] define adapter state and generic transport binding
- [x] implement recv/send/close callbacks without exposing socket details to TLS core
- [x] host socket-pair qualification for bidirectional I/O and owned-socket close
- [ ] AmigaOS/68000 compile qualification of adapter

- [ ] AmigaOS build
- [ ] `amtls.library` lifecycle/API implementation
- [ ] FS-UAE/AROS initial runtime qualification where feasible
- [ ] local AmigaOS 2.04+ qualification

## M4 — CLI toolkit and ARexx automation

- [ ] TLSInfo
- [ ] TLSConnect
- [ ] TLSGet / HTTPS GET
- [ ] TLSBench
- [ ] optional ARexx command layer outside the TLS core
- [ ] `INFO`, `CONNECT`, `GET`, `CERTINFO`, `STATUS`, `VERSION` and `BENCH` commands where applicable
- [ ] documented RC/RESULT2 behavior and example scripts

## M5 — Efficiency qualification

- [ ] A500+/68000 profile
- [ ] A1200/68020 profile
- [ ] 68030/68060 profiles
- [ ] code-size measurements
- [ ] resident and peak RAM measurements
- [ ] handshake timing
- [ ] throughput measurements
- [ ] controlled comparison with AmiSSL where feasible

## Release gate

No production release before certificate validation, hostname verification, entropy handling, protocol policy and real AmigaOS runtime qualification pass.
