# Stage 10A - Public Kernel Allocation API Wrappers

Status: complete

## Goal

Establish a thin public kernel allocation API wrapper layer over the existing Stage 8C and Stage 9A allocator interfaces.

## Scope

Included in Stage 10A:

- `kmalloc(size)` public wrapper API.
- `kfree(ptr)` public wrapper API.
- Deterministic Stage 10A self-check markers in early init.

Not included in Stage 10A:

- Allocator redesign.
- New allocation policy.
- Stage 10B, Stage 10C, Stage 10D, or Stage 10E work.

## Wrapper Contract

- `kmalloc(size)` wraps `stage8c_kheap_alloc(size)`.
- `kfree(ptr)` wraps `stage9a_kheap_free(ptr)`.

## Validation Results (QEMU)

- `kmalloc` allocation succeeded.
- `kfree` valid free result: PASS.
- `kmalloc` exact-reuse result: PASS.
- `kfree` invalid free rejection result: PASS.
- `kfree` double free rejection result: PASS.
- Stage 10A: PASS.
- Stage 9 output remained intact.
- Stage 8 output remained intact.
- Stage 7 output remained intact.
- Stage 6 timer and keyboard runtime remained intact.

## Completion Statement

Stage 10A is complete and validated. Stage 10 overall remains in progress, with Stage 10B and later not started.
