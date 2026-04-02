# Stage 8B - Minimal VMM Mapping Interface

Status: complete

## Goal

Add a minimal, inspectable Stage 8B mapping interface for single-page operations while preserving Stage 7 and Stage 8A behavior.

## Scope

Included in Stage 8B:

- New minimal VMM API for page-sized operations:
  - query whether a virtual page is mapped
  - resolve the mapped physical frame
  - map one virtual page to one physical frame
  - unmap one virtual page
- Stage 8B self-check in early init with deterministic marker output.
- Mapping test constrained to Stage 8A future mapping reserved region.

Not included in Stage 8B:

- Heap/kmalloc.
- Multi-page mapping APIs.
- Higher-half mapping.
- Paging model redesign.
- Stage 8C or Stage 8D work.

## Integration Point

Stage 8B self-check runs in early init:

- after Stage 8A layout policy self-check
- before IDT/PIC/PIT runtime initialization and steady-state IRQ behavior

## Deterministic Marker Set

Stage 8B emits marker lines covering:

- custom-os Stage 8B mapping interface begin
- custom-os Stage 8B initial unmapped state: PASS|FAIL
- custom-os Stage 8B successful map result: PASS|FAIL
- custom-os Stage 8B resolved physical frame: <hex>
- custom-os Stage 8B successful unmap result: PASS|FAIL
- custom-os Stage 8B final unmapped state: PASS|FAIL
- custom-os Stage 8B: PASS|FAIL

## PASS Criteria

- Test virtual page in future mapping reserved region starts unmapped.
- Single-page map operation succeeds.
- Resolved physical frame matches expected frame.
- Single-page unmap operation succeeds.
- Test virtual page returns to unmapped state.

## Validation Results

- Build succeeded in local development environment.
- Boot succeeded in QEMU.
- Stage 7A through Stage 7D output remained present.
- Stage 8A output remained present.
- Stage 8B markers emitted with PASS outcomes.
- Stage 6 timer and keyboard runtime behavior remained intact.

## Files Touched

- kernel/include/mm/vmm.h
- kernel/mm/vmm.c
- kernel/include/mm/paging.h
- kernel/mm/paging.c
- kernel/init/stage0_main.c
- build/config.mk
- build/build.mk
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-8b.md
