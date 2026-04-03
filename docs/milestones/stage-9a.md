# Stage 9A - Heap Free Groundwork And Allocation Tracking

Status: complete

## Goal

Introduce minimal, deterministic heap allocation tracking and a safe free groundwork layer on top of the Stage 8C bump allocator, without enabling block reuse.

## Scope

Included in Stage 9A:

- Metadata recorded per heap allocation.
- Minimal free API: `stage9a_kheap_free(void* ptr)`.
- Deterministic acceptance of valid free requests for tracked allocation starts.
- Deterministic rejection of invalid free pointers.
- Deterministic rejection of double free attempts.
- Deterministic Stage 9A self-check marker set.

Not included in Stage 9A:

- Reuse of freed blocks.
- Free-list activation.
- Fragmentation-aware splitting or coalescing.
- Allocator redesign beyond the Stage 8C bump allocation path.
- Stage 9B, Stage 9C, or Stage 9D behavior.

## Integration Point

Stage 9A self-check runs in early init:

- after Stage 8D heap validation self-check
- before IDT/PIC/PIT runtime setup

## Deterministic Marker Set

- custom-os Stage 9A heap free validation begin
- custom-os Stage 9A alloc A: 0x????????
- custom-os Stage 9A alloc B: 0x????????
- custom-os Stage 9A free A accept: PASS|FAIL
- custom-os Stage 9A duplicate free reject: PASS|FAIL
- custom-os Stage 9A invalid free reject: PASS|FAIL
- custom-os Stage 9A null free reject: PASS|FAIL
- custom-os Stage 9A heap current before free: 0x????????
- custom-os Stage 9A heap current after free: 0x????????
- custom-os Stage 9A alloc C after free: 0x????????
- custom-os Stage 9A no-reuse monotonic alloc result: PASS|FAIL
- custom-os Stage 9A: PASS|FAIL

## Validation Results

- Build succeeded in the validated environment.
- Boot succeeded in QEMU.
- Stage 7A/7B/7C/7D outputs remained intact.
- Stage 8A/8B/8C/8D outputs remained intact.
- Stage 9A markers emitted in deterministic order.
- Valid free was accepted.
- Duplicate free was rejected.
- Invalid free was rejected.
- Null free was rejected.
- No-reuse monotonic allocation check passed.
- Final Stage 9A marker reported PASS.
- Stage 6 timer and keyboard runtime remained intact.

## Completion Statement

Stage 9A is complete and independently verified. Stage 9B and Stage 9C are complete. Stage 9D remains not started.

## Files Touched

- kernel/include/mm/kheap.h
- kernel/mm/kheap.c
- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-9a.md
