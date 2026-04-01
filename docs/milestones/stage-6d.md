# Stage 6D - Deterministic Reuse Activation and Validation

Status: complete

## Goal

Activate frame reuse from Stage 6C pending-free tracking with minimal deterministic behavior and no allocator redesign.

## Scope

Included in Stage 6D:

- Reuse activation from existing pending-free tracking.
- FIFO ordering for reused frames.
- Reused-frame allocation preference before fresh allocation fallback.
- Minimal already-issued check in free path.
- Preserve exported API signatures from Stage 6B and Stage 6C.

Not included in Stage 6D:

- Bitmap allocator redesign.
- Buddy allocator.
- Contiguous multi-frame allocation.
- Full ownership ledger redesign.
- Paging, page tables, heap/kmalloc, scheduler/tasking, user mode, boot path redesign.
- Stage 7 work.

## Reuse Policy Implemented

- `stage6b_pmm_alloc_frame` first attempts pending-free reuse in FIFO order.
- If no pending reuse frame is available, allocation falls back to the existing fresh Stage 6A path.
- `stage6c_pmm_free_frame` keeps Stage 6C validation and adds a minimal already-issued check.

## Determinism Contract

- Pending frees are consumed in insertion order (FIFO).
- Fresh allocation order remains unchanged as fallback.
- Baseline Stage 5D behavior remains unchanged when reuse is not exercised.

## Dedicated Reuse Self-Test Path

Compile-time flag:

- `STAGE6D_FORCE_REUSE_TEST=1`

Behavior:

- Disabled (`0`): normal baseline boot path remains unchanged.
- Enabled (`1`): runs a small deterministic serial-visible self-test after Stage 5D boot allocation output.

Self-test checks:

- valid free acceptance for two known allocated frames (A then B)
- duplicate free rejection
- never-issued eligible frame rejection
- FIFO reuse verification (next two allocations return A then B)

## Validation Expectations

- Build and boot succeed.
- Stage 5A/5B/5C and Stage 5D baseline output remains stable when no frees are injected.
- Stage 5D allocated frame addresses remain unchanged in baseline flow.
- Stage 4 timer and keyboard IRQ output continues after Stage 5D.
- Reuse behavior checks:
  - free A then B, allocate twice => A then B
  - unaligned/out-of-policy/non-eligible/double free => rejected
  - minimal already-issued check rejects never-issued frame frees

- Dedicated self-test markers should report PASS/FAIL per check and overall.

## Validation Results

- Validation date: 2026-04-01
- Baseline boot path remained correct.
- Stage 5A/5B/5C output remained intact.
- Stage 5D deterministic allocation output remained intact.
- Stage 4 timer and keyboard IRQ runtime continued after Stage 5D.
- Dedicated Stage 6D reuse self-test ran in QEMU with `STAGE6D_FORCE_REUSE_TEST=1`.
- PASS markers observed for: alloc A/B, free A accept, free B accept, duplicate free reject, never-issued reject, pending count after free, fifo reuse order, pending drained, overall.
- No regressions were observed.

## Next Step

Stage 6 is complete. Next work is Stage 7 planning only (paging groundwork definition), with no Stage 7 implementation started.
