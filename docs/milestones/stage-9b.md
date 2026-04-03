# Stage 9B - Free-List Reuse Activation

Status: complete

## Goal

Activate deterministic reuse of previously freed heap blocks on top of the verified Stage 9A allocation metadata and free-validation groundwork.

## Scope

Included in Stage 9B:

- Deterministic reuse search in allocation path before bump growth.
- Exact-size freed-block reuse activation.
- Preservation of Stage 9A free-validation behavior.
- Dedicated deterministic Stage 9B self-check markers.

Not included in Stage 9B:

- Fragmentation-aware handling.
- Block splitting.
- Block coalescing.
- Allocator redesign beyond Stage 9A/9B reuse activation.
- Stage 9C or Stage 9D behavior.

## Integration Point

Stage 9B self-check runs in early init:

- after Stage 9A free-groundwork self-check
- before IDT/PIC/PIT runtime setup

## Deterministic Marker Set

- custom-os Stage 9B free-list reuse activation begin
- custom-os Stage 9B alloc A: 0x????????
- custom-os Stage 9B alloc B: 0x????????
- custom-os Stage 9B free A accept: PASS|FAIL
- custom-os Stage 9B reuse allocation result: 0x????????
- custom-os Stage 9B reused-address match: PASS|FAIL
- custom-os Stage 9B: PASS|FAIL

## Validation Results

- Build succeeded in the validated environment.
- Boot succeeded in QEMU.
- Stage 7A/7B/7C/7D outputs remained intact.
- Stage 8A/8B/8C/8D outputs remained intact.
- Stage 9A outputs remained intact.
- Stage 9B markers emitted in deterministic order.
- Free A was accepted.
- Reuse allocation succeeded.
- Reused address matched the freed block address.
- Stage 6 timer and keyboard runtime remained intact.

## Completion Statement

Stage 9B is complete and independently verified. Stage 9C and Stage 9D are complete. Stage 9 aggregate is complete. Stage 10 is not started.

## Files Touched

- kernel/mm/kheap.c
- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-9b.md
