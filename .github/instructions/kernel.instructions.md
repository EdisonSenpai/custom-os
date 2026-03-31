---
description: "Use when creating or modifying kernel, architecture, memory-manager, scheduler, or low-level runtime code for the custom OS. Covers linker, memory layout, ABI safety, and validation."
applyTo:
  - "kernel/**"
  - "arch/**"
  - "include/**"
  - "**/*.c"
  - "**/*.cc"
  - "**/*.cpp"
  - "**/*.cxx"
  - "**/*.h"
  - "**/*.hpp"
  - "**/*.S"
  - "**/*.s"
  - "**/*.ld"
---

# Kernel Development Instructions

## Safety First
- Prefer minimal, reversible edits.
- Confirm architecture assumptions before changing low-level code.
- Ask for approval before changing kernel architecture, virtual memory model, scheduler design, or syscall ABI.

## Boot-to-Kernel Contract
- Keep kernel entry assumptions explicit: CPU mode, paging state, stack pointer, interrupt state, and handoff data format.
- Verify entry symbol names and section placement stay aligned with linker and bootloader expectations.

## Linker and Memory Layout
- Keep linker scripts, section attributes, and startup code synchronized.
- Do not move kernel base addresses, stack regions, page-table regions, or section order without explicit approval.
- Use explicit alignment for page tables, descriptor tables, and interrupt structures.

## ABI-Sensitive Edits
When touching interfaces across privilege boundaries, translation units, or language boundaries:
- Use fixed-width integer types.
- Audit calling conventions, packing, and alignment.
- Add compile-time layout checks where possible.
- Document compatibility impact.

## C and C++ Guidance
- Prefer C for early init paths, hardware-facing routines, and deterministic low-level logic.
- Use C++ in kernel subsystems only when runtime behavior is explicit and controlled.
- Avoid hidden runtime dependencies in early boot paths.

## Validation Checklist
- Build with warnings enabled and treat new warnings as issues.
- Verify entry points, sections, and exported symbols using binary inspection tools.
- Run emulator boot smoke tests when available.
- If automated tests are unavailable, provide manual validation steps with expected boot milestones.
