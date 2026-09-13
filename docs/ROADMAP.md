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

- [ ] define internal TLS backend interface
- [ ] evaluate and pin compact TLS backend (BearSSL first candidate)
- [ ] define transport callback API
- [ ] implement deterministic host test transport
- [ ] establish memory/allocation instrumentation
- [ ] add 68000 cross-compile CI smoke build

## M2 — TLS client handshake

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

## M4 — CLI toolkit

- [ ] TLSInfo
- [ ] TLSConnect
- [ ] TLSGet / HTTPS GET
- [ ] TLSBench
- [ ] ARexx integration where useful

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
