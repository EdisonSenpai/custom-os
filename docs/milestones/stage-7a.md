# Stage 7A - Paging Groundwork (Non-Activating)

Status: complete

## Goal

Introduce minimal, deterministic paging groundwork helpers for 32-bit non-PAE, 4 KiB pages, while keeping runtime paging disabled.

## Scope

Included in Stage 7A:

- New paging header/source split with explicit constants, masks, types, and flags.
- Address decomposition helpers for page directory index, page table index, and page offset.
- Entry-construction helper that masks frame address and flags.
- Identity-map helper for page-aligned address translation in identity-mapped design.
- Deterministic serial self-check output in early init using sample address `0x12345000`.

Not included in Stage 7A:

- CR0/CR3 writes or paging activation.
- Page fault handler extensions.
- Page directory/table allocation or installation.
- Heap/kmalloc, VMM, scheduler/tasking, or user mode work.

## Constraints Kept Explicit

- 32-bit paging model only.
- Non-PAE assumptions only.
- 4 KiB pages only.
- Identity-mapped design only.

## Deterministic Self-Check

Sample values used:

- Virtual address sample: `0x12345000`
- Expected PD index: `0x00000048`
- Expected PT index: `0x00000345`
- Expected page offset: `0x00000000`
- Entry source sample (unaligned): `0x12345ABC`
- Entry with `PRESENT|WRITABLE` expected output: `0x12345003`

PASS/FAIL marker is emitted without changing control-register or interrupt behavior.

## PMM Boundary

PMM remains responsible for physical frame availability and lifecycle decisions.

Stage 7A paging groundwork is responsible only for virtual-address decomposition and paging-entry encoding policy.

Stage 7A does not redefine PMM behavior from Stage 6.

## Validation Results

- Build succeeded.
- Boot succeeded in QEMU.
- Stage 7A self-check markers were emitted.

Observed values:

- Sample virtual address: `0x12345000`
- PD index output: `0x00000048`
- PT index output: `0x00000345`
- Page offset output: `0x00000000`
- Entry source output: `0x12345ABC`
- Masked entry output: `0x12345003`
- Self-check result: `PASS`

Runtime validation:

- Stage 5A/5B/5C outputs remained intact
- Stage 5D allocation behavior unchanged
- Stage 6 timer ticks continued
- Stage 6 keyboard input remained functional
- No paging activation occurred (no CR0/CR3 changes)

## Files Touched

- kernel/include/mm/paging.h
- kernel/mm/paging.c
- kernel/init/stage0_main.c
- build/config.mk
- build/build.mk
- docs/milestone-tracker.md
- docs/milestones/README.md
