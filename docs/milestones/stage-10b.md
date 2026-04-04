# Stage 10B - Allocation Semantics and Invariants

Status: complete

## Goal

Define and validate explicit public `kmalloc` and `kfree` semantics at the wrapper layer without redesigning allocator internals.

## Scope

Included in Stage 10B:

- Explicit `kmalloc(0)` behavior.
- Explicit `kfree(NULL)` behavior.
- Validation that normal `kmalloc(size > 0)` and `kfree(valid_ptr)` behavior remains correct.
- Validation that invalid free rejection remains intact.
- Deterministic Stage 10B serial markers.

Not included in Stage 10B:

- Any `kheap` allocator redesign.
- New allocation-fit policy.
- Stage 10C, Stage 10D, or Stage 10E implementation work.

## Public Semantics

- `kmalloc(0)` returns `NULL`.
- `kfree(NULL)` returns reject/failure (`0`).
- `kmalloc(size > 0)` continues to allocate from existing allocator behavior.
- `kfree(valid_ptr)` continues to free via existing allocator behavior.
- Invalid interior/free mismatch rejection remains intact.

## Deterministic Marker Set

- `custom-os Stage 10B allocation semantics begin`
- `custom-os Stage 10B kmalloc zero-size result: PASS|FAIL`
- `custom-os Stage 10B kfree null result: PASS|FAIL`
- `custom-os Stage 10B kmalloc valid result: <hex>`
- `custom-os Stage 10B kfree valid result: PASS|FAIL`
- `custom-os Stage 10B invalid free rejection result: PASS|FAIL`
- `custom-os Stage 10B: PASS|FAIL`

## Validation Results (QEMU, 2026-04-04)

- Build succeeded (`make kernel`, `make run`).
- `kmalloc(0)` result: PASS.
- `kfree(NULL)` result: PASS.
- `kmalloc` valid result emitted as a non-null address (`0x00401850` in validated run).
- `kfree` valid result: PASS.
- Invalid free rejection result: PASS.
- Final Stage 10B marker: PASS.
- Stage 10A output remained intact.
- Stage 9 output remained intact.
- Stage 8 output remained intact.
- Stage 7 output remained intact.
- Stage 6 timer and keyboard runtime remained intact after Stage 10B.

## Completion Statement

Stage 10B is complete and verified. Stage 10 remains in progress, and Stage 10C and later are not started.
