# Stage 5A: Multiboot2 Memory Map Parsing

Status: in-progress (implemented, pending runtime verification)

## Objective

Implement safe parsing of the Multiboot2 memory map tag without starting allocator work.

## Scope

Included in Stage 5A:

- Locate and walk Multiboot2 tags safely.
- Locate the memory map tag.
- Parse memory map entries.
- Distinguish usable and reserved regions.
- Print per-entry base, length, and type to COM1 serial.
- Print concise summary counts and usable-byte total.

Explicitly not included yet:

- Stage 5B memory accounting policy beyond concise summary output.
- Stage 5C physical frame allocator groundwork.
- Stage 5D frame allocation tests.
- Paging, heap, kmalloc, or allocator APIs.

## Test Procedure

```sh
make clean
make run
```

Observe serial output after Stage 4 init markers.

## Expected Serial Markers

- custom-os Stage 5A: parsing memory map
- custom-os Stage 5A mmap index: 0x...
- base/len/type lines per entry
- custom-os Stage 5A mmap entries: 0x...
- custom-os Stage 5A usable regions: 0x...
- custom-os Stage 5A reserved regions: 0x...
- custom-os Stage 5A usable bytes: 0x...

## Notes For Stage 5B

Stage 5A provides validated raw region data and a usable-memory aggregate that Stage 5B can build on for explicit RAM accounting policy.
