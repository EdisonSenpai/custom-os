# Stage 6A - PMM Internal Cleanup (Behavior Preserving)

Status: complete

## Goal

Stage 6A refines the internal structure of the current early-boot PMM allocator path without changing verified Stage 5 behavior.

## Scope

Included in Stage 6A:

- Formalize PMM allocator state with explicit internal fields and invariants.
- Separate PMM core state-transition logic from the Stage 5D boot smoke-test wrapper.
- Keep deterministic frame allocation order and existing boot-time allocation test flow.
- Preserve Stage 4 runtime continuation after Stage 5D output.

Not included in Stage 6A:

- Stage 6B, Stage 6C, or Stage 6D implementation features.
- Paging or page-table work.
- Heap or kmalloc.
- Scheduler/tasking or user mode.
- Frame free/reuse support.
- Broad public allocation API surface changes.

## Structural Changes Implemented

Primary implementation file:

- kernel/init/stage0_main.c

Changes made:

- Stage 5D allocator internals were reorganized under Stage 6A PMM-focused naming.
- PMM state fields were made more explicit for initialization, range traversal, next-frame cursor, and allocation accounting.
- PMM core helpers now handle state reset, state seeding from Stage 5C bookkeeping, eligible-range advancement, and single-frame allocation.
- Stage 5D smoke-test wrapper remains as the boot-time verification shell and still owns the Stage 5D serial output markers.

## Behavior Preservation Requirements

The following behaviors are intentionally preserved:

- Deterministic allocation order by range order and frame index.
- Existing Stage 5D serial marker text and output sequence.
- Existing Stage 5 boot flow ordering before IDT/PIC/PIT runtime setup.
- Existing Stage 4 IRQ runtime behavior after Stage 5D smoke-test output.

## Validation and Results

Stage 6A was validated successfully in QEMU with the following observed outcomes:

- Build succeeded.
- Boot succeeded.
- Stage 5A, Stage 5B, and Stage 5C behavior and output remained intact.
- Stage 5D deterministic allocation output and marker ordering remained intact.
- Stage 4 timer and keyboard IRQ continuation remained intact after Stage 5D output.
- No regressions were observed.

## Next Step

Stage 6B is the next milestone, limited to planning and design only (scope definition, interface design, and validation planning). Stage 6B implementation work has not started.
