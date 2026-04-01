# Milestones Index

- stage-0.md: complete, repository and build baseline.
- stage-1.md: complete, deterministic init and panic path.
- stage-2.md: complete, IDT and CPU exception groundwork with forced INT3 validation.
- stage-3.md: complete, timer IRQ groundwork (PIC remap, IRQ0, PIT tick signaling, EOI handling).
- stage-4.md: complete, keyboard IRQ groundwork (IRQ1 gate, raw scancode serial output, timer and keyboard IRQ coexistence).
- stage-5a.md: complete, Multiboot2 memory map parsing and serial summary.
- stage-5b.md: complete, memory accounting and usable RAM summary.
- stage-5c.md: complete, physical frame bookkeeping groundwork and serial frame summary markers.

Current focus: Stage 5D frame allocation behavior and validation.

Stage 5 is split into 5A, 5B, 5C, and 5D. Stages 5A through 5C are complete. Full Stage 5 remains incomplete until Stage 5D is verified.
