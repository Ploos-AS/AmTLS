# M3.1 Qualification — bsdsocket-compatible transport adapter

Status: **PASS**

## Scope

M3.1 introduces the platform transport adapter that binds an already-connected bsdsocket-compatible socket to the backend-independent `AmTLS_Transport` interface.

Qualified behavior:

- adapter state and transport callback binding
- bidirectional `recv()` / `send()` transport I/O
- owned-socket close lifecycle
- host socket-pair qualification
- Motorola 68000 cross-compile smoke qualification

The TLS core remains independent of bsdsocket-specific details.

## Evidence

GitHub Actions CI run **#79**, run ID **34974540214**, completed successfully for commit `d0c49a95a9dafa9dc8ca6861b77cd5d426a086b6`.

The `m68k-smoke` target explicitly compiles `src/platform/bsdsocket_transport.c` with `m68k-linux-gnu-gcc -m68000`, in addition to the existing core, allocator, handshake pump and CLI smoke objects.

The host `make check` path runs `tests/test_m3_1_bsdsocket.c`, which verifies bidirectional socket I/O and owned-socket close behavior.

## Qualification boundary

This milestone proves the adapter is host-tested and 68000-source-compatible in the existing cross-compile smoke environment. It does **not** claim real AmigaOS bsdsocket.library runtime qualification. Native AmigaOS build/runtime and FS-UAE qualification remain M3 work and are required by the release gate.

## Result

M3.1: **PASS**.
