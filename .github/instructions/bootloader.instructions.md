---
description: "Use when creating or modifying bootloader, early startup assembly, boot protocol handoff, image layout, or firmware entry flow for the custom OS."
applyTo:
  - "boot/**"
  - "bootloader/**"
  - "startup/**"
  - "arch/**"
  - "**/*.S"
  - "**/*.s"
  - "**/*.asm"
  - "**/*.ld"
---

# Bootloader Development Instructions

## Boot Flow Discipline
- Treat each boot stage as a contract with explicit input and output state.
- Keep CPU mode transitions, stack setup, and paging transitions explicit and documented.
- Do not introduce implicit assumptions about firmware behavior.

## Handoff Contract
- Keep handoff structures explicit, versioned where practical, and validated before kernel jump.
- Define ownership of memory maps, framebuffer data, and boot parameters clearly.
- Preserve backward compatibility or provide a migration note when changing handoff formats.

## Image and Linker Safety
- Keep boot image format, linker script, and entry symbols synchronized.
- Do not change load addresses, entry offsets, stage boundaries, or relocation assumptions without explicit approval.
- Ensure section alignment and memory regions match architecture requirements.

## Toolchain and ABI Controls
- Preserve assembler, linker, and objcopy behavior unless a change is required.
- Treat calling convention boundaries between assembly and C or C++ as ABI-critical.
- Re-verify register and stack conventions after edits.

## Validation Checklist
- Build boot artifacts and verify output sections and entry points.
- Inspect binaries with tools such as readelf, objdump, and nm when relevant.
- Run emulator boot smoke tests and confirm expected milestones.
- Report exact failure stage for boot regressions: image load, entry, transition, or handoff.
