# Stage 7C - Paging Activation Path (Static Identity Map)

Status: complete

## Goal

Add a minimal, inspectable paging activation path that explicitly:

- Loads CR3 with the static Stage 7B page-directory frame.
- Sets CR0.PG to enable paging.
- Emits deterministic serial markers before and after activation.
- Performs a post-enable PASS/FAIL check.

## Scope

Included in Stage 7C:

- New paging symbols for activation/control-register reads:
  - `STAGE7C_CR0_PG_MASK`
  - `stage7c_load_cr3_with_early_page_directory`
  - `stage7c_set_cr0_paging_enable`
  - `stage7c_read_cr0`
  - `stage7c_read_cr3`
- Early-init Stage 7C activation self-check integration in `stage0_main`.
- Deterministic serial output markers for expected and observed CR3/CR0 values.
- Panic on Stage 7C activation check failure.

Not included in Stage 7C:

- Higher-half mappings.
- Dynamic page-table allocation or runtime remapping.
- Stage 7D identity/fault validation logic.
- VMM, heap integration, scheduler/tasking, or user mode changes.

## Integration Point

The Stage 7C activation self-check is called in `stage0_main`:

- After Stage 7A and Stage 7B self-check calls.
- Before IDT/PIC/PIT runtime/IRQ startup flow.

## Deterministic Markers

Serial markers include:

- `custom-os Stage 7C: paging activation begin`
- `custom-os Stage 7C expected CR3   : ...`
- `custom-os Stage 7C observed CR3   : ...`
- `custom-os Stage 7C observed CR0   : ...`
- `custom-os Stage 7C activation: PASS|FAIL`

PASS criteria:

- Observed CR3 frame equals expected static Stage 7B directory frame.
- Observed CR0 has `STAGE7C_CR0_PG_MASK` set.

## Constraints Preserved

- 32-bit non-PAE model only.
- 4 KiB pages only.
- Static first-4 MiB identity mapping only.
- No higher-half transition.
- No dynamic mappings.

## PMM and Paging Boundary

PMM remains responsible for physical frame availability and lifecycle.
Stage 7C only activates paging using the already prepared static Stage 7B identity-mapped structures.
Stage 7C does not introduce dynamic mappings, allocator changes, or VMM behavior.

## Validation Results

- Build succeeded.
- Boot succeeded in QEMU.
- Stage 7A output remained present.
- Stage 7B output remained present.
- Stage 7C activation markers were emitted and PASS.
- Observed expected CR3: `0x0010B000`
- Observed CR3: `0x0010B000`
- Observed CR0: `0x80000011`
- Observed Stage 7C activation result: `PASS`
- Stage 6 timer runtime remained intact after paging enable.
- Stage 6 keyboard IRQ runtime remained intact after paging enable.

## Files Touched

- kernel/include/mm/paging.h
- kernel/mm/paging.c
- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-7c.md
