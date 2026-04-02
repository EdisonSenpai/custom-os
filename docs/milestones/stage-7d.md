# Stage 7D - Identity-Mapping Validation and Fault-Aware Verification

Status: complete

## Goal

Validate Stage 7 active paging behavior after Stage 7C activation using deterministic, non-destructive checks:

- CR0.PG is still set under active paging.
- CR3 still points to the Stage 7B early page-directory frame.
- First-4 MiB identity mappings are readable at known safe addresses.
- Page-fault awareness is confirmed without triggering a forced fault.

## Scope

Included in Stage 7D:

- New Stage 7D self-check routine in early init after Stage 7C.
- Deterministic Stage 7D serial markers for:
  - validation begin
  - observed CR3 under active paging
  - observed CR0 under active paging
  - identity-map probe results
  - page-fault-awareness confirmation
  - Stage 7D PASS/FAIL
- Read-only identity probes at:
  - `0x00001000`
  - `0x000B8000` (VGA text memory)
- Non-destructive page-fault-awareness confirmation by verifying:
  - `0x00400000` is outside the first-4 MiB map
  - PDE[1] remains not present in Stage 7B static directory

Not included in Stage 7D:

- Forced-fault testing.
- New mappings beyond the Stage 7B first-4 MiB identity map.
- Higher-half, dynamic mapping, heap, scheduler, or Stage 8 work.

## Deterministic Marker Set

Stage 7D emits the following deterministic marker lines:

- `custom-os Stage 7D validation begin`
- `custom-os Stage 7D observed CR3 under active paging: ...`
- `custom-os Stage 7D observed CR0 under active paging: ...`
- `custom-os Stage 7D identity-map probe results`
- `custom-os Stage 7D page-fault-awareness confirmation: PASS (guarded, no forced fault)`
- `custom-os Stage 7D validation: PASS`

Additional deterministic support lines are emitted for probe addresses/PTE/read values and page-fault-awareness address/PDE state.

## PASS Criteria

- `(observed_cr0 & STAGE7C_CR0_PG_MASK) != 0`
- `frame(observed_cr3) == frame(stage7b early page directory)`
- identity probe addresses are in PD index 0 and expected PTE entries match static identity map
- page-fault-awareness guard holds:
  - `stage7a_paging_pd_index(0x00400000) == 1`
  - `0x00400000 > 0x003FFFFF`
  - `PDE[1] == 0`

## Validation Results

- Build succeeded.
- Boot succeeded in QEMU.
- Stage 7A/7B/7C output remained present.
- Stage 7D markers emitted and PASS.
- Observed CR3 under active paging: `0x0010B000`
- Observed CR0 under active paging: `0x80000011`
- Identity probes validated at `0x00001000` and `0x000B8000`.
- Fault-awareness guard confirmed for `0x00400000` with `PDE[1] = 0`.
- No forced fault was triggered during Stage 7D validation.
- Stage 6 timer/keyboard runtime behavior remained intact after Stage 7D.
- Stage 7 is finalized by Stage 7D.

## Files Touched

- kernel/init/stage0_main.c
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-7d.md
- README.md
