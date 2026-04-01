# Milestones Index

- stage-0.md: complete, repository and build baseline.
- stage-1.md: complete, deterministic init and panic path.
- stage-2.md: complete, IDT and CPU exception groundwork with forced INT3 validation.
- stage-3.md: complete, timer IRQ groundwork (PIC remap, IRQ0, PIT tick signaling, EOI handling).
- stage-4.md: complete, keyboard IRQ groundwork (IRQ1 gate, raw scancode serial output, timer and keyboard IRQ coexistence).
- stage-5a.md: complete, Multiboot2 memory map parsing and serial summary.
- stage-5b.md: complete, memory accounting and usable RAM summary.

Current focus: hold before Stage 5C allocator groundwork approval.

Stage 5 is split into 5A, 5B, 5C, and 5D. Do not treat full Stage 5 as complete until all sub-stages are verified.
