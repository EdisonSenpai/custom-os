# Stage 8A - VMM Layout Policy Baseline

Status: complete

## Goal

Define a minimal, explicit Stage 8A virtual memory layout policy baseline that is inspectable and deterministic, while preserving Stage 7 paging behavior and Stage 6 runtime behavior.

## Scope

Included in Stage 8A:

- Policy constants for three virtual layout windows:
  - active identity-mapped range (first 4 MiB)
  - future reserved heap range
  - future reserved mapping expansion range
- 4 KiB alignment/invariant declarations for the policy boundaries.
- Address and region classification helpers with enum and string-friendly category output.
- Deterministic Stage 8A self-check routine in early init with PASS/FAIL marker output.

Not included in Stage 8A:

- Dynamic mappings.
- Map/unmap APIs.
- Heap allocator or kmalloc activation.
- Higher-half transition.
- Paging activation redesign.
- Stage 8B, Stage 8C, or Stage 8D work.

## Integration Point

Stage 8A self-check runs in early init:

- after Stage 7D identity validation self-check
- before IDT/PIC/PIT runtime initialization and steady-state IRQ loop behavior

## Deterministic Marker Lines

Expected Stage 8A marker lines:

- custom-os Stage 8A: layout policy begin
- custom-os Stage 8A active identity-mapped range
- custom-os Stage 8A future reserved heap range
- custom-os Stage 8A future reserved mapping range
- custom-os Stage 8A address classification examples
- custom-os Stage 8A: PASS

Additional deterministic support lines are emitted for:

- active start/end addresses
- future heap start/end addresses
- future mapping start/end addresses
- sample address and observed classification name

## Validation Results

- Build succeeded in the local development environment.
- Boot succeeded in QEMU.
- Stage 7A, Stage 7B, Stage 7C, and Stage 7D output remained present.
- Stage 8A markers were emitted and PASS.

Observed runtime markers:

- Active identity start: `0x00000000`
- Active identity end: `0x003FFFFF`
- Future heap start: `0x00400000`
- Future heap end: `0x007FFFFF`
- Future mapping start: `0x00800000`
- Future mapping end: `0x00FFFFFF`

Observed classification examples:

- `0x00001000` -> `active-identity`
- `0x00400000` -> `future-heap-reserved`
- `0x00800000` -> `future-mapping-reserved`
- `0x01000000` -> `unclassified`

- Stage 6 timer and keyboard runtime remained intact after Stage 8A.

## Files Touched

- kernel/include/mm/vmm_layout.h
- kernel/mm/vmm_layout.c
- kernel/init/stage0_main.c
- build/config.mk
- build/build.mk
- docs/milestone-tracker.md
- docs/milestones/README.md
- docs/milestones/stage-8a.md
