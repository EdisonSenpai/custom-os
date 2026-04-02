# Milestones Index

- stage-0.md: complete, repository and build baseline.
- stage-1.md: complete, deterministic init and panic path.
- stage-2.md: complete, IDT and CPU exception groundwork with forced INT3 validation.
- stage-3.md: complete, timer IRQ groundwork (PIC remap, IRQ0, PIT tick signaling, EOI handling).
- stage-4.md: complete, keyboard IRQ groundwork (IRQ1 gate, raw scancode serial output, timer and keyboard IRQ coexistence).
- stage-5a.md: complete, Multiboot2 memory map parsing and serial summary.
- stage-5b.md: complete, memory accounting and usable RAM summary.
- stage-5c.md: complete, physical frame bookkeeping groundwork and serial frame summary markers.
- stage-5d.md: complete, deterministic boot-time frame allocation smoke test.
- stage-6a.md: complete, PMM internal cleanup and state formalization with behavior-preserving validation.
- stage-6b.md: complete, PMM public API surface validated in QEMU with behavior preserved and no regressions.
- stage-6c.md: complete, minimal PMM free API and deterministic pending-free tracking validated in QEMU with no allocation-path regressions.
- stage-6d.md: complete, deterministic pending-free FIFO reuse activation validated in QEMU, including dedicated self-test PASS markers.
- stage-7a.md: complete, non-PAE 4 KiB paging groundwork helpers and deterministic non-activating self-check markers.
- stage-7b.md: complete, static 4 KiB-aligned paging aggregates and deterministic first-4 MiB identity-map setup self-check with paging still disabled.
- stage-7c.md: complete, explicit CR3 load and CR0.PG activation path validated in QEMU with continued execution and no Stage 6 runtime regressions.
- stage-7d.md: complete, active paging validation with deterministic CR3/CR0 markers, first-4 MiB identity probes, and non-destructive page-fault-awareness confirmation.

Current focus: Stage 7 complete (7A through 7D), verified in QEMU with active paging baseline validation in place. Stage 8 planning only (not started).

Stage 5 is split into 5A, 5B, 5C, and 5D. All Stage 5 sub-stages are complete, so full Stage 5 is complete.
Stage 6 is split into 6A, 6B, 6C, and 6D. All Stage 6 sub-stages are complete and validated.
Stage 7 is split into 7A, 7B, 7C, and 7D. All Stage 7 sub-stages are complete, so full Stage 7 is complete.
