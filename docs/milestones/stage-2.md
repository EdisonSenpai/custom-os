# Stage 2: Exceptions and IDT Groundwork

## Objective

Introduce a minimal IDT and exception dispatch path suitable for early bring-up debugging.

## Deliverables

- [x] IDT install path added during deterministic init.
- [x] Core exception stubs wired for early bring-up:
  - #DE divide error
  - #BP breakpoint
  - #UD invalid opcode
  - #GP general protection
  - #PF page fault
- [x] Exception dispatch prints diagnostics to VGA and COM1 serial.
- [x] Forced INT3 validation path added.
- [x] Exception path halts safely after reporting.

## Exit Criteria

- Normal boot reaches Stage 2 deterministic init messages.
- Forced INT3 reliably reaches exception display path.
- Exception diagnostics include vector, error code, and saved control context.

## Test Commands

Normal path:

```sh
make clean
make run
```

Forced INT3 path:

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

## Deferred To Stage 3

- PIC remapping for IRQ-safe vector space.
- Timer IRQ groundwork and periodic signal output.
- IRQ end-of-interrupt handling.
