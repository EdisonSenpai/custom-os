# Boot Assumptions and Handoff Notes (Stage 0)

Purpose: define the boot-to-kernel contract before writing low-level code.

## Entry assumptions (Stage 0 baseline)

- Bootloader: GRUB with Multiboot2 kernel loading.
- CPU mode at entry: 32-bit protected mode.
- Interrupt state at entry: treated as disabled on entry path.
- Stack state at entry: explicitly initialized by stage0_entry.S.
- Paging state at entry: no paging assumptions used in Stage 0.
- Register contract used in entry:
	- eax: Multiboot2 magic.
	- ebx: Multiboot2 information structure address.

Note: Stage 0 validates pipeline and handoff shape only. Full long-mode transition is deferred.

## Handoff contract (current)

- Boot stage is expected to provide Multiboot2 info tags.
- Stage 0 kernel currently accepts magic and info address but does not parse full tag set yet.
- Stage 1 will add explicit required-tag validation and parse checks.

## Ownership and lifetime notes

- Stage 0 treats Multiboot2 data as bootloader-owned and read-only.
- No boot memory reclamation is performed in Stage 0.

## Linker and entry-point assumptions

- Linker script: linker/stage0.ld.
- ELF class: 32-bit (Stage 0 only).
- Entry symbol: stage0_start.
- Multiboot2 header section: .multiboot, aligned to 8 bytes, kept first via linker script.
- Load base: 1 MiB.

## Validation notes

- Verify entry symbols and sections with binary inspection tools.
- Emulator milestones for Stage 0:
	- GRUB menu appears.
	- Kernel entry reached.
	- VGA text marker is visible.

## Change control

- Any change here should be reflected in architecture and linker planning docs.
