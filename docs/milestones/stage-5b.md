# Stage 5B: Memory Accounting and Usable RAM Summary

Status: complete

## Objective

Build a clear accounting view from Stage 5A memory map data without introducing allocation behavior.

## Scope

Included in Stage 5B:

- Preserve parsed memory region bookkeeping in fixed-size structures.
- Summarize raw usable bytes.
- Summarize raw reserved bytes.
- Summarize policy-usable bytes for future allocation policy.
- Summarize policy-unavailable bytes within raw-usable ranges.
- Summarize highest policy-usable region end.
- Keep output concise and serial-first for deterministic debugging.

Explicitly not included:

- Stage 5C physical frame allocator groundwork.
- Stage 5D allocation tests.
- Paging, virtual memory, heap, or allocator APIs.

## Deliverables

- [x] Stage 5A parser and Stage 5B accounting boundary made explicit in code.
- [x] Accounting summaries emitted with Stage 5B-prefixed serial markers.
- [x] Region-storage-cap semantics clarified (totals remain authoritative over all parsed entries).

## Test Procedure

```sh
make clean
make run
```

Observe serial output after Stage 4 init and before interrupt-driven steady-state output.

## Expected Serial Output Markers

- custom-os Stage 5A: parsing memory map
- custom-os Stage 5A mmap entries: 0x...
- custom-os Stage 5B regions stored: 0x...
- custom-os Stage 5B regions dropped: 0x...
- custom-os Stage 5B raw usable bytes: 0x...
- custom-os Stage 5B raw reserved bytes: 0x...
- custom-os Stage 5B policy usable bytes: 0x...
- custom-os Stage 5B policy unavailable bytes: 0x...
- custom-os Stage 5B highest usable end: 0x...

## Validation Result

- Validation date: 2026-04-01
- Stage 5B accounting output is present and coherent.
- Stage 4 timer and keyboard IRQ behavior remains active after initialization.

## Notes For Stage 5C

Stage 5B now provides a stable accounting baseline and bounded region bookkeeping that Stage 5C can consume when introducing physical frame allocator groundwork.
