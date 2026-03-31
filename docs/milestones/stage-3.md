# Stage 3: Timer IRQ Groundwork

Status: complete

## Objective

Add minimal timer interrupt groundwork with PIC remapping, IRQ0 handling, and periodic serial tick output.

## Deliverables

- [x] PIC remapped to IRQ vectors 0x20-0x2F.
- [x] IDT gate installed for IRQ0 timer at vector 0x20.
- [x] Minimal IRQ0 assembly stub added.
- [x] C IRQ0 handler increments a tick counter.
- [x] Periodic serial tick output demonstrates recurring interrupts.
- [x] EOI sent to PIC after IRQ handling.

## Exit Criteria

- Stage 3 init messages appear during boot.
- System continues to receive timer interrupts while idle in halt loop.
- Serial output shows recurring tick updates.
- No freeze after first interrupt (EOI path working).

## Test Commands

Normal Stage 3 run:

```sh
make clean
make run
```

Optional Stage 2 exception regression check:

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

## Expected Runtime Output

Normal path should include:

- custom-os Stage 3: init start
- custom-os Stage 3: Multiboot2 handoff OK
- custom-os Stage 3: IDT installed
- custom-os Stage 3: PIC remapped + PIT started
- custom-os Stage 3: deterministic init OK
- recurring serial lines like custom-os Stage 3 tick: 0x00000064

## Validation Notes

- Validation date: 2026-04-01
- `make clean && make run` shows Stage 3 startup markers, PIC/PIT initialization text, and recurring serial tick output while idle.
- `make clean && make run STAGE2_FORCE_EXCEPTION=1` remains available as a regression check for the Stage 2 exception diagnostics path.
- Stage 3 exit criteria are satisfied: interrupts continue over time, and no freeze occurs after the first timer interrupt.

## Deferred To Stage 4+

- Keyboard IRQ handling.
- Scheduler and timeslice logic.
- Additional IRQ sources and masking policy.
