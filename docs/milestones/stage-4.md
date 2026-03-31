# Stage 4: Keyboard IRQ Groundwork

Status: complete

## Objective

Add minimal keyboard interrupt groundwork while preserving existing timer IRQ behavior.

## Goals

- Add keyboard IRQ1 support.
- Install IRQ1 keyboard gate in the IDT.
- Keep IRQ0 timer behavior active.
- Read raw scancodes from port 0x60.
- Print scancodes to COM1 serial in hex.
- Acknowledge keyboard IRQ with EOI.

## Deliverables

- [x] IRQ1 keyboard stub wired in assembly.
- [x] IDT entry installed for keyboard interrupt vector 0x21.
- [x] PIC mask updated to keep IRQ0 and IRQ1 unmasked.
- [x] Minimal C handler reads one-byte scancode from port 0x60.
- [x] Scancode values logged to COM1 serial.
- [x] EOI sent after keyboard interrupt handling.

## Test Procedure

Normal Stage 4 run:

```sh
make clean
make run
```

In QEMU, press a few keys while observing serial output.

Optional Stage 2 exception regression check:

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

## Validation Result

- Validation date: 2026-04-01
- Timer IRQ0 remains active with recurring serial tick output.
- Keyboard IRQ1 triggers on key presses and emits raw scancode values to serial.
- Timer and keyboard IRQ handlers coexist correctly in the interruptible idle loop.
- No regressions observed in Stage 2 exception handling path.

## Deferred To Stage 5+

- Keymap translation and key state tracking.
- Input buffering and line discipline.
- Shell/editor input integration.
