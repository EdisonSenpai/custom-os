# Stage 8C - Heap Bootstrap Allocator Groundwork

Status: complete

## Fix Update

Observed runtime failure was caused by Stage 8B mutable mapping policy accepting only the Stage 8A future mapping reserved window, while Stage 8C heap bootstrap maps pages in the Stage 8A future heap reserved window.

Stage 8C fix scope applied:

- Stage 8B VMM mutable mapping slot resolution now accepts controlled single-page mappings in both policy-approved windows:
  - Stage 8A future heap reserved range (`0x00400000` to `0x007FFFFF`)
  - Stage 8A future mapping reserved range (`0x00800000` to `0x00FFFFFF`)
- No Stage 8D work added.
- No heap free logic added.
- No broader allocator or paging-activation redesign introduced.

## Goal

Add a minimal, deterministic heap bootstrap layer that uses the Stage 8A future heap reserved window and Stage 8B single-page mapping interface, while preserving Stage 7 paging behavior and Stage 6 runtime behavior.

## Scope

Included in Stage 8C:

- New minimal heap bootstrap API:
  - `stage8c_kheap_bootstrap_init`
  - `stage8c_kheap_alloc`
  - `stage8c_kheap_get_state`
- Heap window bound to Stage 8A future heap reserved range:
  - start: `0x00400000`
  - end exclusive: `0x00800000`
- Bump-only allocator behavior:
  - minimum 8-byte alignment
  - deterministic monotonic pointer growth
  - no free path
- On-demand page backing using one-page-at-a-time growth:
  - physical frame allocation via `stage6b_pmm_alloc_frame`
  - page mapping via `stage8b_vmm_map_page`
- Deterministic Stage 8C self-check markers in early init.

Not included in Stage 8C:

- `free` support.
- General-purpose `kmalloc` subsystem.
- Multi-page allocation APIs.
- Higher-half mapping.
- Stage 8D behavior.
- Paging activation flow changes.

## Integration Point

Stage 8C self-check runs in early init:

- after Stage 8B mapping interface self-check
- before runtime IDT/PIC/PIT setup

## Deterministic Marker Set

Stage 8C emits marker lines covering:

- custom-os Stage 8C heap bootstrap begin
- custom-os Stage 8C heap start: <hex>
- custom-os Stage 8C heap current: <hex>
- custom-os Stage 8C heap end exclusive: <hex>
- custom-os Stage 8C heap mapped end exclusive: <hex>
- custom-os Stage 8C first allocation result: <hex>
- custom-os Stage 8C second allocation result: <hex>
- custom-os Stage 8C alignment/ordering check: PASS|FAIL
- custom-os Stage 8C: PASS|FAIL

Additional deterministic support lines are emitted for post-allocation current and mapped-end values.

## Validation Results

- Build succeeded in the local development environment.
- Boot succeeded in QEMU.
- Stage 7A through Stage 7D output remained present.
- Stage 8A output remained present.
- Stage 8B output remained present.
- Stage 8C markers emitted and PASS.
- Heap bootstrap state initialized correctly:
  - heap start: `0x00400000`
  - heap current (initial): `0x00400000`
  - heap end exclusive: `0x00800000`
  - heap mapped end exclusive (initial): `0x00400000`
- Allocation results:
  - first allocation: `0x00400000`
  - second allocation: `0x00400018`
  - heap current post-allocation: `0x00400040`
  - heap mapped end post-allocation: `0x00401000`
- Alignment and ordering check: `PASS`
- Stage 6 timer and keyboard runtime remained intact.

## Validation Checklist

- [x] Build with `make kernel` succeeds.
- [x] Boot in QEMU succeeds.
- [x] Stage 8A and Stage 8B markers remain unchanged and PASS.
- [x] Stage 8C markers emit and PASS (expected corrected marker flow after mapping-gate fix).
- [x] Stage 7 behavior remains intact.
- [x] Stage 6 timer/keyboard runtime behavior remains intact.

## Files Touched

- kernel/include/mm/kheap.h
- kernel/mm/kheap.c
- kernel/init/stage0_main.c
- build/config.mk
- build/build.mk
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-8c.md
