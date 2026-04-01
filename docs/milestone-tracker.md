# Milestone Tracker

Purpose: one-page status view for solo progress.

## Status legend

- not-started
- in-progress
- blocked
- complete

## Milestones

| Milestone | Status | Target date | Exit criteria summary | Evidence |
| --- | --- | --- | --- | --- |
| Stage 0 - Build and run baseline | complete | TBD | make iso and make run succeed with Stage 0 marker | Build log plus QEMU capture |
| Stage 1 - Deterministic init and panic path | complete | TBD | Multiboot2 magic validated, clear success and panic output on VGA and serial | Success run log plus forced panic run log |
| Stage 2 - Exceptions and IDT groundwork | complete | TBD | IDT installed, core exception dispatch works, forced INT3 reaches diagnostics | Exception screen plus serial capture |
| Stage 3 - Timer IRQ groundwork | complete | TBD | PIC remap + IRQ0 + PIT ticks produce recurring serial tick output without freeze | Serial tick capture over time |
| Stage 4 - Keyboard IRQ groundwork | complete | TBD | IRQ1 receives keyboard events, raw scancodes printed to serial, timer and keyboard IRQs coexist | Serial capture showing ticks and scancodes |
| Stage 5A - Multiboot2 memory map parsing | in-progress | TBD | Memory map tag located and parsed with serial region summary output | Serial capture with parsed entries and usable summary |

## Current focus

- Verify Stage 5A memory map parsing output in QEMU.
- Keep Stage 5B/5C/5D out of scope until Stage 5A is validated.

## Weekly update template

- Week of: 2026-04-01

- Completed:
    -- Stage 5A implemented (Multiboot2 memory map parsing)
    -- Safe tag walking + validation
    -- Serial output for memory regions

-Blockers:  -- None

-Next focus:
    -- Validate Stage 5A output in QEMU
    -- Prepare Stage 5B memory accounting

-Risk changes:
    -- Memory parsing verified as stable base for allocator work
