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

## Current focus

- Keep Stage 4 interrupt behavior stable and retain regression checks for Stage 2 and Stage 3 paths.
- Define Stage 5 scope before introducing new runtime subsystems.

## Weekly update template

- Week of: YYYY-MM-DD
- Completed:
- Blockers:
- Next focus:
- Risk changes:
