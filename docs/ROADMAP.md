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
- [ ] pin reviewed BearSSL source revision before functional TLS integration

## M2 — TLS client handshake

- [ ] pin/vendor reviewed BearSSL revision with provenance and integrity hash
- [ ] TLS client state machine
- [ ] SNI/server-name handling
- [ ] CA trust loading strategy suitable for classic Amiga
- [ ] certificate-chain validation
- [ ] hostname verification
- [ ] negative/security tests

## M3 — Amiga network integration

- [ ] bsdsocket-compatible transport adapter
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
