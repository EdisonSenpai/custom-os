# Stage 10D - Public API Lifecycle Hardening Validation

Status: complete

## Goal

Add a deterministic validation-only lifecycle hardening self-check through the public `kmalloc` and `kfree` API, without redesigning allocator internals.

## Scope

Included in Stage 10D:

- Multiple `kmalloc` allocations and ordering/alignment validation.
- Exact-size reuse validation through `kmalloc` and `kfree`.
- Split-fragment reuse validation through `kmalloc` and `kfree`.
- Invalid free rejection validation through `kfree`.
- Double free rejection validation through `kfree`.
- Deterministic Stage 10D serial markers and PASS/FAIL gate.

Not included in Stage 10D:

- Allocator redesign.
- New allocation policy.
- New allocation APIs (`calloc`, `realloc`, statistics subsystem).
- Stage 10E work.

## Deterministic Marker Set

- `custom-os Stage 10D allocator hardening validation begin`
- `custom-os Stage 10D alloc A: <hex>`
- `custom-os Stage 10D alloc B: <hex>`
- `custom-os Stage 10D alloc C: <hex>`
- `custom-os Stage 10D exact-size reuse candidate: <hex>`
- `custom-os Stage 10D exact-size reuse result: PASS|FAIL`
- `custom-os Stage 10D split-reuse candidate: <hex>`
- `custom-os Stage 10D split-reuse result: PASS|FAIL`
- `custom-os Stage 10D leftover-fragment candidate: <hex>`
- `custom-os Stage 10D leftover-fragment result: PASS|FAIL`
- `custom-os Stage 10D invalid free rejection result: PASS|FAIL`
- `custom-os Stage 10D double free rejection result: PASS|FAIL`
- `custom-os Stage 10D alignment/ordering result: PASS|FAIL`
- `custom-os Stage 10D: PASS|FAIL`

## Validation Results (QEMU, 2026-04-04)

- Build succeeded.
- Stage 10D begin marker emitted.
- Alloc A/B/C markers emitted with non-null, increasing, aligned addresses.
- Exact-size reuse candidate emitted and exact-size reuse result: PASS.
- Split-reuse candidate emitted and split-reuse result: PASS.
- Leftover-fragment candidate emitted and leftover-fragment result: PASS.
- Invalid free rejection result: PASS.
- Double free rejection result: PASS.
- Final Stage 10D marker: PASS.
- Stage 10A output remained intact.
- Stage 10B output remained intact.
- Stage 10C output remained intact.
- Stage 9 output remained intact.
- Stage 8 output remained intact.
- Stage 7 output remained intact.
- Stage 6 timer and scancode runtime remained intact after Stage 10D.

## Completion Statement

Stage 10D is complete and verified. Stage 10 remains in progress. Stage 10E is not started.
