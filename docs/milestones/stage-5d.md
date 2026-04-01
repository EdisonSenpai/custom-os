# Stage 5D: Deterministic Frame Allocation Boot Test

Status: complete

## Objective

Introduce a minimal, deterministic early-boot frame allocation path that consumes Stage 5C frame ranges and proves basic allocation behavior without adding allocator lifecycle complexity.

## Scope

Included in Stage 5D:

- Initialize a tiny allocator state from Stage 5C frame ranges.
- Allocate a small fixed number of frames during boot (4 requests).
- Emit allocated physical addresses to COM1 serial.
- Emit remaining eligible-frame count after the fixed allocation pass.
- Preserve existing boot path and Stage 4-origin IRQ runtime behavior.

Explicitly not included:

- Free or deallocation semantics.
- Paging, virtual memory, heap, kmalloc, or allocator APIs.
- Long-lived allocator integration outside deterministic boot validation.

## Implementation Notes

- Stage 5D consumes `g_stage5c_frames` as the source of eligible ranges.
- Allocation policy is first-fit in range order with a monotonic frame cursor.
- State is fixed-size static bookkeeping only.

## Expected Serial Output Markers

- custom-os Stage 5D: deterministic frame allocation test
- custom-os Stage 5D alloc request count: 0x...
- custom-os Stage 5D alloc index: 0x...
- custom-os Stage 5D alloc addr : 0x...
- custom-os Stage 5D alloc granted: 0x...
- custom-os Stage 5D remaining eligible frames: 0x...

## Test Procedure

```sh
make clean
make run
```

Capture serial output and confirm Stage 5D markers appear before Stage 5 steady-state IRQ output.

## Exit Criteria

- [x] Fixed-count Stage 5D boot allocation test implemented.
- [x] Allocated physical addresses emitted deterministically to serial.
- [x] Remaining eligible frame summary emitted after allocations.
- [x] Runtime validation evidence captured (serial log) with no Stage 4 regressions.

## Validation Result

- Validation date: 2026-04-01
- Stage 5D allocation markers are present with deterministic frame addresses.
- Remaining eligible-frame summary is emitted after allocations.
- Stage 4 timer IRQ and keyboard IRQ behavior continues after Stage 5D test output.
