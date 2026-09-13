# AmTLS architecture

## M0 boundary

M0 defines interfaces and constraints only. No TLS protocol or cryptographic implementation is included.

## Layers

### Public API

`include/amtls/amtls.h` is the backend-independent application contract. The long-term Amiga library API should remain small and stable.

### Core

The core owns configuration, connection state, error mapping and lifecycle. It must avoid leaking TLS-engine types into the public API.

### TLS backend

A later milestone will introduce an internal backend interface. BearSSL is the leading candidate because compact embedded implementations fit AmTLS's goals, but M0 deliberately does not lock the project to it.

### Platform/network

Socket and AmigaOS-specific code belongs behind a platform boundary. The public API must not require application code to understand the selected TCP/IP stack.

## Baseline constraints

- AmigaOS 2.04+
- 68000-compatible code generation
- no FPU requirement
- low memory usage
- no mandatory threads
- client-side TLS first
- cross-build friendly

## Security requirements for later milestones

A release must not claim production readiness until it has working certificate-chain validation, hostname verification, secure protocol/cipher policy, entropy handling, error propagation and runtime qualification on target systems.

## Efficiency qualification

Future qualification should record at least:

- executable/library size
- resident memory
- peak memory during handshake
- handshake time
- transfer throughput
- target CPU and TCP/IP stack

Where feasible the same workload should be measured against AmiSSL to keep the project's lightweight goal objective rather than anecdotal.
