# Boot Assumptions and Handoff Notes (Stage 0)

Purpose: define the boot-to-kernel contract before writing low-level code.

## Entry assumptions (to confirm during implementation)

- CPU mode at entry: TODO.
- Interrupt state at entry: TODO.
- Stack state at entry: TODO.
- Paging state at entry: TODO.
- Boot information format: Multiboot2 tags (planned).

## Handoff contract draft

- Boot stage provides: memory map, command line, module info, framebuffer info (if present).
- Kernel stage validates required tags before continuing.
- Kernel stage copies or owns any data needed past early init.

## Ownership and lifetime notes

- Define which boot data is transient versus retained.
- Define when temporary boot memory can be reclaimed.

## Validation notes (future)

- Verify entry symbols and sections with binary inspection tools.
- Add emulator boot milestones for: load, entry, handoff parse, kernel init marker.

## Change control

- Any change here should be reflected in architecture and linker planning docs.