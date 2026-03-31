# Boot Assumptions and Handoff Notes (Stage 1)

Purpose: define the boot-to-kernel contract before writing low-level code.

## Entry assumptions (Stage 1 baseline)

- Bootloader: GRUB with Multiboot2 kernel loading.
- CPU mode at entry: 32-bit protected mode.
- Interrupt state at entry: treated as disabled on entry path.
- Stack state at entry: explicitly initialized by stage0_entry.S.
- Paging state at entry: no paging assumptions used in Stage 1.
- Register contract used in entry:
  - eax: Multiboot2 magic.
  - ebx: Multiboot2 information structure address.

Note: Stage 1 keeps the same bootstrap path and focuses on deterministic early init and fatal-path handling. Full long-mode transition is deferred.

## Handoff contract (current)

- Boot stage is expected to provide Multiboot2 info tags.
- Stage 1 kernel explicitly validates the Multiboot2 magic value (`0x36D76289`).
- A null Multiboot2 information pointer is treated as a fatal early boot error.
- Stage 1 still does not parse the full tag set; detailed tag validation remains future work.

## Ownership and lifetime notes

- Stage 1 treats Multiboot2 data as bootloader-owned and read-only.
- No boot memory reclamation is performed in Stage 1.

## Linker and entry-point assumptions

- Linker script: linker/stage0.ld.
- ELF class: 32-bit (Stage 0 only).
- Entry symbol: stage0_start.
- Multiboot2 header section: .multiboot, aligned to 8 bytes, kept first via linker script.
- Load base: 1 MiB.

## Validation notes

- Verify entry symbols and sections with binary inspection tools.
- Emulator milestones for Stage 1:
  - GRUB menu appears.
  - Kernel entry reached.
  - Stage 1 init status lines appear on VGA and serial.
  - Invalid handoff enters panic path and halts.

## Change control

- Any change here should be reflected in architecture and linker planning docs.
