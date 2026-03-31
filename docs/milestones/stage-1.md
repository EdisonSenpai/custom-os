# Stage 1: Deterministic Init and Panic Path

## Objective

Add deterministic early init sequencing and a minimal fatal panic path while preserving the existing Stage 0 boot flow.

## Deliverables

- [x] Explicit Stage 1 init order in kernel entry:
  - clear VGA text buffer
  - initialize COM1 serial
  - emit init start marker
  - validate Multiboot2 magic and non-null info pointer
  - emit success markers
- [x] Minimal panic mechanism for fatal early boot errors.
- [x] Panic messages emitted to both VGA and serial.
- [x] Existing assembly boot path and halt behavior preserved.

## Exit Criteria

- Normal boot path prints clear Stage 1 success lines to VGA and serial.
- Invalid Multiboot2 magic or null info pointer enters panic path.
- Panic path halts cleanly and does not continue initialization.

## Test Commands

Success path:

```sh
make clean
make run
```

Forced panic path:

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

## Expected Runtime Output

Success path (VGA and serial):

- custom-os Stage 1: init start
- custom-os Stage 1: Multiboot2 handoff OK
- custom-os Stage 1: deterministic init OK

Forced panic path (VGA and serial):

- custom-os Stage 1 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Deferred To Stage 2

- Exception and interrupt handler bring-up.
- Richer Multiboot2 tag parsing policy.
- Timer-based diagnostics.
