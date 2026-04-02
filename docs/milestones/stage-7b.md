# Stage 7B - Static Paging Structures and Setup Groundwork

Status: complete

## Goal

Introduce deterministic static paging structures for early boot setup in the 32-bit non-PAE 4 KiB model, while keeping paging inactive.

## Scope

Included in Stage 7B:

- Concrete aggregate paging types:
  - page_table_t
  - page_directory_t
- 4 KiB alignment guarantees for both aggregate types.
- Compile-time assertions for structure size and alignment.
- One static early page directory in paging.c.
- One static early identity page table in paging.c.
- Deterministic setup routine that:
  - Clears all page-directory entries.
  - Clears all page-table entries.
  - Fills 1024 PTEs for first 4 MiB identity mapping.
  - Sets PDE[0] to the identity page table with PRESENT|WRITABLE.
- Deterministic Stage 7B serial self-check markers for:
  - page_table_t size
  - page_directory_t size
  - first PTE
  - last PTE
  - PDE[0]

Not included in Stage 7B:

- CR3 writes.
- CR0 paging enable writes.
- Page fault path expansion.
- Dynamic page-table allocation.
- VMM, heap, scheduler, tasking, or user mode work.

## Deterministic Self-Check Expectations

Expected values:

- page_table_t size: 0x00001000
- page_directory_t size: 0x00001000
- first PTE: 0x00000003
- last PTE: 0x003FF003
- PDE[0]: identity-page-table frame address ORed with 0x00000003

PASS marker is emitted when all checks match.

## Constraints Preserved

- 32-bit non-PAE paging model only.
- 4 KiB page size only.
- Identity-mapped setup only.
- Paging remains disabled in runtime flow.

## PMM and Paging Boundary

PMM remains responsible for physical frame availability and lifecycle.
Stage 7B paging setup uses static, deterministic mapping structure construction only.
Stage 7B does not change PMM allocation/free semantics from Stage 6.

## Validation Results

- Build succeeded.
- Boot succeeded in QEMU.
- Stage 7A output remained present.
- Stage 7B self-check markers were emitted and PASS.
- Observed page_table_t size: `0x00001000`
- Observed page_directory_t size: `0x00001000`
- Observed first PTE: `0x00000003`
- Observed last PTE: `0x003FF003`
- Observed PDE[0]: `0x0010C003`
- Observed Stage 7B self-check: `PASS`
- No CR3 writes were introduced.
- No CR0 paging-enable writes were introduced.
- Stage 6 timer and keyboard runtime behavior remained intact.

## Files Touched

- kernel/include/mm/paging.h
- kernel/mm/paging.c
- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-7b.md
