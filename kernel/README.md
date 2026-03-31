# Kernel Notes

Current status: Stage 4 early kernel groundwork is active.

Current implementation location:

- init/stage0_main.c: deterministic bring-up path used from entry to interrupt-driven idle operation.

Current capabilities:

- Early init sequence with Multiboot2 entry validation.
- Panic path output to VGA and COM1 serial.
- IDT install and exception dispatch diagnostics.
- PIC remap and PIT setup for recurring timer IRQ0.
- Keyboard IRQ1 raw scancode capture to COM1 serial.
- Interruptible idle state with periodic timer tick output.

Not implemented yet:

- Scheduler and task switching.
- Memory management subsystem.
- Filesystem and user-mode runtime.
- Full keyboard input processing pipeline.
