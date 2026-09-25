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
