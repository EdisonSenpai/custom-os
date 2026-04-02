# Stage 8D - Heap Validation And Controlled Allocation Tests

Status: complete

## Goal

Finalize Stage 8 by validating the Stage 8C heap bootstrap behavior with deterministic, inspectable allocation checks under the active Stage 7 paging baseline.

## Scope

Included in Stage 8D:

- Dedicated Stage 8D self-check routine in early init.
- Deterministic allocation-validation markers.
- Multiple controlled bump allocations using existing Stage 8C API.
- Validation of:
  - non-null allocation results
  - aligned and strictly increasing addresses
  - mapped-end page-growth advancement
  - heap-window bounds compliance
- Final Stage 8D PASS/FAIL gate.

Not included in Stage 8D:

- Free logic (`free`, `kfree`).
- Allocator redesign (slab/buddy/fragmentation handling).
- API expansion beyond current Stage 8C bump allocator model.
- Stage 9 work.

## Integration Point

Stage 8D self-check runs in early init:

- after Stage 8C heap bootstrap self-check
- before IDT/PIC/PIT runtime setup

## Deterministic Marker Set

- custom-os Stage 8D allocation validation begin
- custom-os Stage 8D heap state before test start: <hex>
- custom-os Stage 8D heap state before test current: <hex>
- custom-os Stage 8D heap state before test end exclusive: <hex>
- custom-os Stage 8D heap state before test mapped end: <hex>
- custom-os Stage 8D allocation index: <hex>
- custom-os Stage 8D allocation size: <hex>
- custom-os Stage 8D allocation result: <hex>
- custom-os Stage 8D heap state after test current: <hex>
- custom-os Stage 8D heap state after test mapped end: <hex>
- custom-os Stage 8D multiple allocation results: PASS|FAIL
- custom-os Stage 8D ordering and alignment result: PASS|FAIL
- custom-os Stage 8D page-growth / mapped-end advancement result: PASS|FAIL
- custom-os Stage 8D heap-window bounds result: PASS|FAIL
- custom-os Stage 8D: PASS|FAIL

## Validation Results

- Build succeeded in the validated environment.
- Boot succeeded in QEMU.
- Stage 7A/7B/7C/7D output remained present.
- Stage 8A/8B/8C output remained present.
- Stage 8D allocation validation markers emitted.
- Stage 8D checks passed:
  - non-null allocation results
  - aligned and monotonic addresses
  - mapped-end advancement beyond first mapped page
  - heap-window bounds retained
- Stage 6 timer and keyboard runtime remained intact.

## Completion Statement

Stage 8 is finalized by Stage 8D. Stage 9 remains not started.

## Files Touched

- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-8d.md
