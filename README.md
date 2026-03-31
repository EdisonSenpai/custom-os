# custom-os

Stage 0 repository scaffold for a from-scratch custom OS project.

## Stage 0 status

- Focus: repository shape, decision records, and build/run placeholders.
- Current state: no real kernel implementation yet.
- Goal: make the project easy to grow safely and incrementally.

## Scope for Stage 0

- Define a clean folder structure for boot, linker, kernel, and architecture areas.
- Capture architecture decisions and boot handoff assumptions.
- Add milestone planning and progress tracking docs.
- Add placeholder build metadata and QEMU run scripts.

## Non-goals for Stage 0

- No full boot chain or kernel implementation.
- No scheduler, memory manager, or syscall implementation.
- No ABI-sensitive code beyond placeholder notes.

## Repository map

- docs/: planning and decision documents.
- docs/milestones/: milestone definitions and stage checklists.
- boot/: boot flow and loader-facing placeholders.
- arch/x86_64/: architecture-specific placeholders.
- linker/: linker-script planning placeholders.
- kernel/: kernel subsystem placeholders.
- build/: build variable and target placeholders.
- scripts/: run and inspection script placeholders.

## Stage 0 acceptance criteria

- Repository structure is clean and extensible.
- Decisions are documented before implementation.
- Boot assumptions and handoff contract are explicit.
- Milestone tracking is available in-repo.
- Build and QEMU entry points exist as placeholders.

## Quick start (placeholder phase)

1. Review docs/architecture-decisions.md.
2. Review docs/boot-handoff-notes.md.
3. Update docs/milestones/stage-0.md tasks as you complete setup.
4. Replace placeholder build and run files with real commands during Stage 1.

## Next stage

Move from planning to first bootable artifact only after Stage 0 checklist is complete.