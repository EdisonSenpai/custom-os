# Stage 9C - Fragmentation-Aware Block Handling

Status: complete

## Goal

Extend Stage 9A/9B heap reuse so larger freed blocks can satisfy smaller aligned allocations with deterministic, minimal split-capable behavior.

## Scope

Included in Stage 9C:

- Deterministic freed-block scan in heap order.
- Reuse of freed blocks when payload size is greater than or equal to aligned request size.
- Exact-size reuse behavior from Stage 9B preserved.
- Split of oversized freed blocks when leftover can form a valid fragment.
- Full-block consume fallback when leftover fragment would be invalid.
- Dedicated deterministic Stage 9C self-check markers.

Not included in Stage 9C:

- Coalescing adjacent free blocks.
- Advanced fit strategies.
- Full fragmentation repair policies.
- Stage 9D lifecycle validation suite behavior.

## Split Rule

For a freed block with payload_size >= aligned request size:

- If remainder >= (allocation header size + minimum aligned payload), split:
  - reuse front portion for allocation
  - emit leftover fragment as a new freed tracked block
- Otherwise, consume entire freed block without splitting.

## Integration Point

Stage 9C self-check runs in early init:

- after Stage 9B reuse activation self-check
- before IDT/PIC/PIT runtime setup

## Deterministic Marker Set

- custom-os Stage 9C fragmentation-aware reuse begin
- custom-os Stage 9C alloc large block: 0x????????
- custom-os Stage 9C free large block accept: PASS|FAIL
- custom-os Stage 9C alloc smaller block result: 0x????????
- custom-os Stage 9C reused-start match: PASS|FAIL
- custom-os Stage 9C leftover fragment allocation: 0x????????
- custom-os Stage 9C leftover fragment reuse result: PASS|FAIL
- custom-os Stage 9C: PASS|FAIL

## Validation Results

- Build succeeded in the validated environment.
- Boot succeeded in QEMU.
- Stage 9C markers emitted in deterministic order.
- Large allocation succeeded.
- Free large block accept reported PASS.
- Smaller allocation reused the same start address as the freed large block.
- Reused-start match reported PASS.
- Leftover fragment allocation succeeded.
- Leftover fragment reuse result reported PASS.
- Final Stage 9C marker reported PASS.
- Stage 9A and Stage 9B outputs remained intact.
- Stage 8A/8B/8C/8D outputs remained intact.
- Stage 7A/7B/7C/7D outputs remained intact.
- Stage 6 timer and keyboard runtime remained intact.

## Completion Statement

Stage 9C is complete and independently verified. Stage 9D is complete. Stage 9 aggregate is complete. Stage 10 is not started.

## Files Touched

- kernel/mm/kheap.c
- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-9c.md
