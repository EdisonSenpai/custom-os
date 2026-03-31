# Custom OS Repository Guidelines

## Mission
- Build a reliable, inspectable custom operating system with predictable behavior.
- Prioritize correctness and debuggability over quick but fragile shortcuts.

## Preferred Languages
- Prefer C and C++ for kernel, memory manager, scheduler, and core runtime code unless the user explicitly requests another language.
- Use assembly only where required: bootstrapping, interrupt and trap stubs, context switches, and architecture entry points.

## Change Discipline
- Inspect the relevant area before editing: bootloader, kernel init, linker script, memory map headers, and build scripts.
- Propose a concise plan before non-trivial implementation.
- Make minimal, controlled edits that preserve existing structure.
- Keep interfaces stable unless a change is explicitly requested.
- Ask for confirmation before large architectural changes, subsystem rewrites, or ABI breaks.

## Boot Flow and Handoff Safety
- Treat boot flow as a strict contract: firmware or bootloader, loader handoff, early kernel entry, architecture init, memory init, then higher-level services.
- Keep handoff structures explicit and versioned where practical.
- Document assumptions about CPU mode, paging state, stack state, and memory map ownership.

## Linker and Memory Layout Rules
- Keep linker scripts, section attributes, and startup code synchronized.
- Do not change load addresses, virtual offsets, page sizes, section order, or kernel base without explicit rationale and user approval.
- Use fixed-width integer types and explicit alignment for layout-critical data.
- Add compile-time checks for structure size and offset where ABI or memory layout is critical.

## ABI-Sensitive Changes
When touching calling conventions, packed structures, interrupt frames, syscall boundaries, or binary interfaces:
- List impacted files and symbols.
- Explain compatibility impact and migration strategy.
- Validate generated symbols and sections with binary inspection tools.

## Toolchain and Build Safety
- Preserve target triple, compiler, linker, and flags unless change is requested.
- Avoid introducing non-deterministic build steps.
- Keep warning levels high. Do not silence warnings globally to pass a build.
- Prefer explicit compiler and linker flags over implicit defaults.

## Validation Expectations
For meaningful code changes:
- Run available build commands and report success or failure.
- Run available emulation or boot smoke tests and report outcomes.
- If boot, linker, or ABI behavior changed, inspect artifacts such as symbol tables, section headers, and entry points, then summarize findings.
- If no automated tests exist, provide concise manual validation steps and observed results.

## Communication and Reporting
After edits:
- Summarize what changed and why.
- List files touched and key risk areas.
- Report validation commands executed and the important outcomes.
- Call out remaining assumptions or follow-up work.