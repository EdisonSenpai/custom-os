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
- stage-8a.md: complete, Stage 8A policy-only virtual layout constants, classification helpers, and deterministic Stage 8A self-check markers.
- stage-8b.md: complete, minimal single-page VMM query/resolve/map/unmap interface in the Stage 8A future mapping reserved window with deterministic Stage 8B self-check markers.
- stage-8c.md: complete, minimal heap bootstrap layer over Stage 8A reserved heap range with Stage 6B PMM-backed and Stage 8B-mapped one-page growth plus deterministic Stage 8C self-check markers.
- stage-8d.md: complete, validation-only controlled heap allocation checks (alignment, monotonic ordering, mapped-end growth, and heap-window bounds) with deterministic Stage 8D PASS/FAIL markers.
- stage-9a.md: complete, heap allocation metadata tracking and minimal free-validation groundwork verified with deterministic Stage 9A PASS markers (no reuse yet).
- stage-9b.md: complete, deterministic exact-size freed-block reuse activation validated with Stage 9B PASS markers while preserving Stage 9A safety checks.
- stage-9c.md: complete, deterministic split-capable reuse for larger freed blocks verified in QEMU with Stage 9C PASS markers.
- stage-9d.md: complete, deterministic lifecycle validation suite over Stage 9A/9B/9C behavior verified in QEMU with Stage 9D PASS markers.
- stage-10a.md: complete, public kernel allocation API wrappers (`kmalloc` and `kfree`) validated in QEMU with deterministic Stage 10A PASS markers.
- stage-10b.md: complete, wrapper-level allocation semantics and invariants (`kmalloc(0)`, `kfree(NULL)`, valid alloc/free, invalid-free rejection) validated in QEMU with deterministic Stage 10B PASS markers.
- stage-10c.md: complete, freestanding memory primitive layer (`kmemset`, `kmemcpy`, overlap-safe `kmemmove`, `kmemcmp`) validated in QEMU with deterministic Stage 10C PASS markers.

Current focus: Stage 10 is split and active. Stage 10A, Stage 10B, and Stage 10C are complete. Stage 10D and later are not started.

Stage 5 is split into 5A, 5B, 5C, and 5D. All Stage 5 sub-stages are complete, so full Stage 5 is complete.
Stage 6 is split into 6A, 6B, 6C, and 6D. All Stage 6 sub-stages are complete and validated.
Stage 7 is split into 7A, 7B, 7C, and 7D. All Stage 7 sub-stages are complete, so full Stage 7 is complete.
Stage 8 is split into 8A, 8B, 8C, and 8D. All Stage 8 sub-stages are complete, so full Stage 8 is complete.
Stage 9 is split into 9A, 9B, 9C, and 9D. All Stage 9 sub-stages are complete, so full Stage 9 is complete.
Stage 10 is split into 10A, 10B, 10C, 10D, and 10E. Stage 10A, Stage 10B, and Stage 10C are complete. Stage 10D and later are not started.
