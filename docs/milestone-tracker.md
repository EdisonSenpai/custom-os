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
| Stage 5D - Deterministic frame allocation boot test | complete | TBD | Fixed-count early-boot frame allocations emit physical addresses and remaining eligible-frame summary without Stage 4 regressions | Serial capture with Stage 5D allocation markers and ongoing Stage 4 IRQ output |
| Stage 5 - Memory groundwork suite (5A-5D) | complete | TBD | All Stage 5 sub-stages verified independently and together | Stage 5A through Stage 5D evidence set |
| Stage 6A - PMM state formalization and internal cleanup | complete | TBD | PMM allocator internals are reorganized with explicit state boundaries while preserving Stage 5D boot-test behavior and outputs | QEMU validation: successful build and boot, intact Stage 5A/5B/5C and Stage 5D markers, ongoing Stage 4 IRQ output, no regressions |
| Stage 6B - PMM public allocation API surface | complete | TBD | Export narrow PMM allocation API (`stage6b_pmm_alloc_frame`, `stage6b_pmm_get_remaining_frames`) while preserving deterministic behavior and Stage 5D smoke-test outputs | QEMU validation: build and boot succeeded; Stage 5A/5B/5C output intact; Stage 5D deterministic output intact; allocated frame addresses unchanged; remaining eligible-frame count correct; Stage 4 timer+keyboard IRQ output continues after Stage 5D; no regressions. |

## Current focus

- Stage 5A through Stage 5D are complete and verified.
- Full Stage 5 is complete.
- Stage 6A internal PMM cleanup is complete and validated.
- Stage 6B PMM public API surface is complete and validated.
- Stage 6C has not started; next work is planning/design only.

## Weekly update template

- Week of: 2026-04-01

- Completed:
  - Stage 5A memory map parsing
  - Stage 5B memory accounting and policy filtering
  - Stage 5C physical frame bookkeeping groundwork
  - Stage 5D deterministic frame allocation boot test (validated)
  - Stage 6A PMM internal cleanup completed and validated (behavior preserved, no regressions)
  - Stage 6B PMM public API surface completed and validated (behavior preserved, no regressions)

- Blockers:
  - None

- Next focus:
  - Stage 6C planning and design only (scope definition, API shape, validation plan)
  - Keep Stage 6C implementation not started
  - Keep frame free/reuse behavior out of scope for now

- Risk changes:
  - Stage 5D allocator path is intentionally minimal and non-freeing; full allocator lifecycle remains future work
  - Stage 6A kept allocator behavior unchanged by design and validated against existing Stage 5D and Stage 4 markers
  - Stage 6B validation completed with no regressions; Stage 6C remains planning-only
