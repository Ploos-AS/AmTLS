# M3.2 Qualification — Native AmigaOS build and library lifecycle

Status: **PASS**

## Scope

M3.2 establishes the native AmigaOS/68000 build contract and the initial `amtls.library` lifecycle ABI.

Qualified behavior:

- native AmigaOS build with `m68k-amigaos-gcc`
- Motorola 68000 / soft-float target
- compiler-native Exec `Resident` / `RTF_AUTOINIT` structure
- library Open, Close, Expunge and reserved vectors
- delayed-expunge lifecycle handling
- native Amiga Hunk library link
- automated host lifecycle tests
- automated native AmigaOS build qualification

## Evidence

GitHub Actions CI run **#94**, run ID **35275409037**, completed successfully for commit `b6ac4f75ea8778ea6ebf08fb6145a5439ee125ab`.

The `amigaos-native` job builds `src/amiga/library_resident.c` with the AmigaOS m68k toolchain and links `build/amigaos/amtls.library` using `-nostdlib -nostartfiles`. The job verifies that the resulting library and TLSInfo artifacts are non-empty and uploads them as CI artifacts.

The host `make check` path includes `tests/test_m3_2_library.c`, covering the portable lifecycle model. The native resident implementation uses the compiler's Exec `struct Resident` definition and provides the standard Open, Close, Expunge and reserved vectors.

## Qualification boundary

M3.2 proves build-time/native-format qualification and lifecycle behavior in automated tests. It does **not** claim that `OpenLibrary()` has yet been exercised under a running classic AmigaOS environment.

Runtime loading under FS-UAE/AROS where feasible and local AmigaOS 2.04+ qualification remain M3.3 requirements. ROMs, Kickstart and proprietary AmigaOS files must not be committed to this repository.

## Result

M3.2: **PASS**.
