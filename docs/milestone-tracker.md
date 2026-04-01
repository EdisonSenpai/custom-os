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
| Stage 5A - Multiboot2 memory map parsing | complete | TBD | Memory map tag located and parsed with serial region summary output | Serial capture with parsed entries and usable summary |
| Stage 5B - Memory accounting / usable RAM summary | complete | TBD | Raw and policy-filtered RAM accounting summaries emitted with stable bookkeeping view | Serial capture with Stage 5B summary markers |
| Stage 5C - Physical frame bookkeeping groundwork | complete | TBD | Policy-usable ranges transformed into 4 KiB frame bookkeeping with stable summary markers | Serial capture with Stage 5C frame summary markers |

## Current focus

- Stage 5A, Stage 5B, and Stage 5C are complete and verified.
- Full Stage 5 is not complete until Stage 5D is implemented and verified.
- Next focus: Stage 5D frame allocation behavior and validation.

## Weekly update template

- Week of: 2026-04-01

- Completed:
  - Stage 5A memory map parsing
  - Stage 5B memory accounting and policy filtering
  - Stage 5C physical frame bookkeeping groundwork

- Blockers:
  - None

- Next focus:
  - Stage 5D allocation behavior and validation

- Risk changes:
  - Stage 5 frame groundwork is stable, but full Stage 5 completion still depends on Stage 5D outcomes
