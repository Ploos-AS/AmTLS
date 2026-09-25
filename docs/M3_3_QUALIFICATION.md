# M3.3 Runtime Qualification

Status: **IN PROGRESS**

M3.3 proves that the native `amtls.library` can be loaded and exercised by a running AmigaOS environment.

## M3.3a — library load/lifecycle smoke

The native test `tests/amiga/test_m3_3_openclose.c`:

1. opens `amtls.library`
2. reports library version/revision
3. closes it
4. repeats OpenLibrary/CloseLibrary 100 times
5. returns AmigaDOS RC 0 only on success

The test is cross-built by the `amigaos-native` CI job.

Build/staging gate: **PASS** (GitHub Actions run #132, commit `e63510b`, including the complete staged M3.3a/b classic qualification payload and bidirectional M3.3b transport smoke). This is build evidence only, not classic AmigaOS runtime evidence.

## M3.3b — bsdsocket DNS/TCP smoke

The native test `tests/amiga/test_m3_3_bsdsocket.c` opens `bsdsocket.library`, resolves `example.com`, establishes a TCP connection to port 80, exercises both the write and read paths of the AmTLS bsdsocket transport, and emits `AMTLS_M3_3B_PASS` on success. The current endpoint is an external smoke-test dependency and is not yet considered deterministic qualification infrastructure.

The staged contract declares `network: true` and `bsdsocket_library: true`; the shared classic FS-UAE backend enables `bsdsocket_library = 1` from that requirement.

## FS-UAE harness

`tools/stage_m3_3_runtime.sh` creates a ROM-free payload.

`tools/run_m3_3_fsuae.sh` prepares and launches an A500+ FS-UAE qualification environment using external, legally supplied inputs:

- `AMTLS_KICKSTART_ROM`
- `AMTLS_SYSTEM_DIR`

No Kickstart ROM, Workbench installation or proprietary AmigaOS file may be committed to this repository.

Inside AmigaOS run:

```
Execute TEST:S/run-test
```

The harness writes `TEST:T/amtls-m3.3.log`. Qualification requires both:

```
RC=0
AMTLS_M3_3_PASS
```

The host-side verifier is:

```sh
tools/verify_m3_3_runtime.sh build/m3_3_fsuae/Test/T/amtls-m3.3.log
```

## Target matrix

Initial gate:

- A500+ / 68000 / AmigaOS 2.x

Follow-up qualification:

- A500 / 68000 / AmigaOS 2.04 where available
- A1200 / 68020 / AmigaOS 3.x

FS-UAE is the first emulator target. Amiberry and FellowNG may be added after the baseline runtime gate.

## PASS rule

Do not mark M3.3 PASS merely because the executable cross-compiles or FS-UAE starts. PASS requires the marker produced by a running AmigaOS instance after the actual library lifecycle test completes successfully.

## Shared amiga-runtime integration

The staged payload includes `amiga-runtime.json` and can be consumed by the shared Ploos-AS `amiga-runtime` qualification service.

For classic AmigaOS, use the private/self-hosted `amiga-classic` runner documented by amiga-runtime. The initial gate is the `a500-os204` profile. Proprietary ROM and AmigaOS assets remain external to both repositories and are mounted read-only at runtime.

M3.3a/b remain **IN PROGRESS** until runtime evidence from that classic guest records the required successful result markers, including `RC=0`, `AMTLS_M3_3_PASS`, and `AMTLS_M3_3B_PASS`.


## Classic runner handoff

Use the staged directory itself as the project payload; do not reconstruct it from individual binaries.

On the private `amiga-classic` runner, obtain the `build/m3_3_runtime/` directory from the successful AmTLS CI artifact and invoke the shared `Ploos-AS/amiga-runtime` workflow **Classic AmigaOS qualification** with:

- `payload_path`: absolute path to that `m3_3_runtime` directory on the runner
- `profile`: `a500-os204`

The runner supplies `AMIGA_RUNTIME_KICKSTART_ROM` and `AMIGA_RUNTIME_SYSTEM_DIR` privately. Neither value nor the referenced files belong in AmTLS artifacts, logs, or the repository.

A successful shared-runtime result must still contain all contract markers:

```
RC=0
AMTLS_M3_3_PASS
AMTLS_M3_3B_PASS
```

The sanitized `classic-amigaos-a500-os204-evidence` artifact from `amiga-runtime` is the evidence to retain for the milestone. Build CI success alone must not be substituted for it.
