# x86_64 Architecture Notes

Current status: Stage 10 complete baseline in 32-bit protected mode with active paging.

Current files in this folder:

- stage0_entry.S: Multiboot2 entry path, stack setup, and fallback halt loop.
- stage2_exceptions.S: exception stubs plus IRQ timer and keyboard stubs.

Multiboot2 header note:

- `stage0_entry.S` defines the Multiboot2 header in an allocatable section: `.section .multiboot, "a"`.

Current architecture responsibilities:

- CPU exception entry stubs for early bring-up diagnostics.
- IRQ entry stubs for timer IRQ0 and keyboard IRQ1.
- Minimal assembly-to-C dispatch glue with simple, explicit control flow.
- Stable handoff into Stage 10 complete baseline runtime in C.

Not implemented in this folder yet:

- Full x86_64 long-mode runtime entry path.
- Context switching and scheduler assembly glue.
- Expanded IRQ coverage beyond current groundwork.
