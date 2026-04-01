# Stage 5C: Physical Frame Bookkeeping Groundwork

Status: complete

## Objective

Convert Stage 5B policy-usable memory regions into 4 KiB frame-oriented bookkeeping suitable for later allocator behavior.

## Scope

Included in Stage 5C:

- Build bounded frame-range bookkeeping from Stage 5B policy-usable regions.
- Align region boundaries to 4 KiB frame granularity.
- Track range count, total eligible frames, highest eligible frame index, and highest eligible frame address.
- Emit concise Stage 5C serial summary markers for deterministic validation.

Explicitly not included:

- Stage 5D frame allocation behavior and allocation-path tests.
- Page allocator APIs, paging policy changes, heap, or kmalloc behavior.

## Deliverables

- [x] Stage 5C frame-range bookkeeping implemented using fixed-size storage.
- [x] Stage 5C serial summary output integrated into early init flow.
- [x] Stage 5C groundwork verified without regressing Stage 4 IRQ behavior.

## Test Procedure

```sh
make clean
make run
```

Observe serial output during early init after Stage 5A parsing.

## Expected Serial Output Markers

- custom-os Stage 5C range count: 0x...
- custom-os Stage 5C eligible frames: 0x...
- custom-os Stage 5C highest eligible frame: 0x...
- custom-os Stage 5C highest eligible address: 0x...

## Validation Result

- Validation date: 2026-04-01
- Stage 5C frame bookkeeping markers are present and coherent.
- Stage 4 timer and keyboard IRQ behavior remains active after initialization.

## Notes For Stage 5D

Stage 5C established the non-allocating frame bookkeeping baseline that Stage 5D now uses for deterministic boot-time frame allocation validation.
