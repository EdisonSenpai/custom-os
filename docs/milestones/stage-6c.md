# Stage 6C - PMM Minimal Free API and Pending-Free Tracking

Status: complete

## Goal

Add a minimal free-facing PMM API surface that validates and records pending frees deterministically, while keeping Stage 6B allocation behavior unchanged.

## Scope

Included in Stage 6C (minimal):

- Export `int stage6c_pmm_free_frame(uint64_t phys_addr)`.
- Export `int stage6c_pmm_get_pending_free_frames(uint64_t* out_count)`.
- Validate free input for alignment and Stage 5C policy-eligible frame range membership.
- Record accepted frees into a fixed-capacity pending-free list in deterministic insertion order.
- Reject duplicate pending-free entries.

Not included in Stage 6C:

- Reuse of freed frames for future allocations.
- Any change to `stage6b_pmm_alloc_frame` allocation order or behavior.
- Bitmap, buddy allocator, or rich free-ledger complexity.
- Ownership or "already issued" verification against a full allocation history.

## API Contract

`int stage6c_pmm_free_frame(uint64_t phys_addr)`:

- Returns `1` when the frame address is accepted into pending-free tracking.
- Returns `0` when rejected due to invalid alignment, out-of-policy/non-eligible frame, duplicate pending entry, or pending list capacity reached.
- Has no impact on current allocation path and does not make the frame reusable yet.

`int stage6c_pmm_get_pending_free_frames(uint64_t* out_count)`:

- Returns `1` on success and writes the current pending-free frame count.
- Returns `0` if `out_count` is null.
- Read-only query with no side effects.

## Implementation Notes

Primary implementation file:

- kernel/init/stage0_main.c

Minimal structure used in Stage 6C:

- Fixed-capacity `uint64_t` array of pending-free frame indices.
- `uint64_t` pending-free count.
- Linear validation scan across Stage 5C ranges for eligibility.
- Linear duplicate check across pending-free entries.

This keeps the implementation small and deterministic for early bring-up.

## Known Limitation (Intentional for Stage 6C)

Stage 6C does not verify that a frame was previously allocated by Stage 6B before accepting it into pending-free tracking. That stronger ownership check is deferred to a later stage to avoid introducing allocation-ledger complexity in this milestone.

## Validation Results

- Build succeeded and boot succeeded in QEMU.
- Stage 5A/5B/5C output remained intact.
- Stage 5D deterministic allocation output remained intact.
- Allocated frame addresses were unchanged.
- Remaining eligible-frame count was correct.
- Stage 4 timer and keyboard IRQ output continued after Stage 5D.
- No regressions observed.

## Scope Boundary

- Stage 6C remains validation-and-record-only for pending frees.
- No frame reuse activation was introduced.
- No allocation-path behavior was changed.
- Stage 6D is unstarted and reserved for planning/design at this time.
