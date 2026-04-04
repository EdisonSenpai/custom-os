# Stage 10C - Memory Utility Primitives

Status: complete

## Goal

Add a minimal freestanding, kernel-owned memory primitive layer for future subsystems without relying on libc and without changing allocator behavior.

## Scope

Included in Stage 10C:

- `kmemset`
- `kmemcpy`
- `kmemmove`
- `kmemcmp`
- Deterministic Stage 10C self-check markers in early init.

Not included in Stage 10C:

- Allocator redesign.
- New allocator policy.
- Stage 10D or Stage 10E work.

## Public Primitive Surface

- `void* kmemset(void* dest, int value, uint32_t count);`
- `void* kmemcpy(void* dest, const void* src, uint32_t count);`
- `void* kmemmove(void* dest, const void* src, uint32_t count);`
- `int kmemcmp(const void* a, const void* b, uint32_t count);`

## Deterministic Marker Set

- `custom-os Stage 10C memory primitives validation begin`
- `custom-os Stage 10C kmemset result: PASS|FAIL`
- `custom-os Stage 10C kmemcpy result: PASS|FAIL`
- `custom-os Stage 10C kmemmove overlap result: PASS|FAIL`
- `custom-os Stage 10C kmemcmp equality result: PASS|FAIL`
- `custom-os Stage 10C kmemcmp difference result: PASS|FAIL`
- `custom-os Stage 10C: PASS|FAIL`

## Validation Results (QEMU, 2026-04-04)

- Build and boot succeeded in the validated environment.
- `kmemset` result: PASS.
- `kmemcpy` result: PASS.
- `kmemmove` overlap result: PASS.
- `kmemcmp` equality result: PASS.
- `kmemcmp` difference result: PASS.
- Final Stage 10C marker: PASS.
- Stage 10A output remained intact.
- Stage 10B output remained intact.
- Stage 9 output remained intact.
- Stage 8 output remained intact.
- Stage 7 output remained intact.
- Stage 6 timer and keyboard runtime remained intact after Stage 10C.

## Completion Statement

Stage 10C is complete and verified. Stage 10 remains in progress. Stage 10D and later are not started.
