# Stage 6B - PMM Public API Surface (Minimal, Behavior Preserving)

Status: complete

## Goal

Expose a narrow PMM allocation API for early kernel use while preserving deterministic Stage 5D behavior and output.

## Scope

Included in Stage 6B:

- Export `int stage6b_pmm_alloc_frame(uint64_t* out_phys_addr)`.
- Export `uint64_t stage6b_pmm_get_remaining_frames(void)`.
- Keep existing Stage 6A internals internal (`stage6a_pmm_*` helpers remain file-local).
- Route Stage 5D smoke-test allocation calls through the Stage 6B API surface.

Not included in Stage 6B:

- Stage 6C or Stage 6D features.
- Rich status/error code expansion.
- Frame free/reuse support.
- Paging, heap, scheduler, or user-mode work.

## API Contract

`int stage6b_pmm_alloc_frame(uint64_t* out_phys_addr)`:

- Returns `1` on success and writes a physical frame address to `out_phys_addr`.
- Returns `0` on failure.
- Does not write output on failure.

`uint64_t stage6b_pmm_get_remaining_frames(void)`:

- Read-only query with no side effects.
- Returns current remaining eligible frame count.

## Implementation Notes

Primary implementation file:

- kernel/init/stage0_main.c

Minimal Stage 6B implementation introduced:

- Added exported Stage 6B wrapper for allocation over existing Stage 6A internal allocator logic.
- Added exported Stage 6B read-only accessor for remaining frame count.
- Updated Stage 5D smoke test to call Stage 6B API, preserving serial markers and control flow.

## Validation Status

Current status: complete.

## Validation Results

- Build succeeded.
- Boot succeeded in QEMU.
- Stage 5A/5B/5C output remained intact.
- Stage 5D deterministic allocation output remained intact.
- Allocated frame addresses remained unchanged from baseline.
- Remaining eligible-frame count remained correct.
- Stage 4 timer and keyboard IRQ runtime continued after Stage 5D.
- No regressions observed.

Stage 6C has not started. Frame free/reuse remains out of scope beyond Stage 6B.
