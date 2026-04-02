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
| Stage 0 - Build and run baseline | complete | 2026-03-31 | make iso and make run succeed with Stage 0 marker | Build log plus QEMU capture |
| Stage 1 - Deterministic init and panic path | complete | 2026-03-31 | Multiboot2 magic validated, clear success and panic output on VGA and serial | Success run log plus forced panic run log |
| Stage 2 - Exceptions and IDT groundwork | complete | 2026-03-31 | IDT installed, core exception dispatch works, forced INT3 reaches diagnostics | Exception screen plus serial capture |
| Stage 3 - Timer IRQ groundwork | complete | 2026-04-01 | PIC remap + IRQ0 + PIT ticks produce recurring serial tick output without freeze | Serial tick capture over time |
| Stage 4 - Keyboard IRQ groundwork | complete | 2026-04-01 | IRQ1 receives keyboard events, raw scancodes printed to serial, timer and keyboard IRQs coexist | Serial capture showing ticks and scancodes |
| Stage 5A - Multiboot2 memory map parsing | complete | 2026-04-01 | Memory map tag located and parsed with serial region summary output | Serial capture with parsed entries and usable summary |
| Stage 5B - Memory accounting / usable RAM summary | complete | 2026-04-01 | Raw and policy-filtered RAM accounting summaries emitted with stable bookkeeping view | Serial capture with Stage 5B summary markers |
| Stage 5C - Physical frame bookkeeping groundwork | complete | 2026-04-01 | Policy-usable ranges transformed into 4 KiB frame bookkeeping with stable summary markers | Serial capture with Stage 5C frame summary markers |
| Stage 5D - Deterministic frame allocation boot test | complete | 2026-04-01 | Fixed-count early-boot frame allocations emit physical addresses and remaining eligible-frame summary without Stage 4 regressions | Serial capture with Stage 5D allocation markers and ongoing Stage 4 IRQ output |
| Stage 5 - Memory groundwork suite (5A-5D) | complete | 2026-04-01 | All Stage 5 sub-stages verified independently and together | Stage 5A through Stage 5D evidence set |
| Stage 6A - PMM state formalization and internal cleanup | complete | 2026-04-01 | PMM allocator internals are reorganized with explicit state boundaries while preserving Stage 5D boot-test behavior and outputs | QEMU validation: successful build and boot, intact Stage 5A/5B/5C and Stage 5D markers, ongoing Stage 4 IRQ output, no regressions |
| Stage 6B - PMM public allocation API surface | complete | 2026-04-01 | Export narrow PMM allocation API (`stage6b_pmm_alloc_frame`, `stage6b_pmm_get_remaining_frames`) while preserving deterministic behavior and Stage 5D smoke-test outputs | QEMU validation: build and boot succeeded; Stage 5A/5B/5C output intact; Stage 5D deterministic output intact; allocated frame addresses unchanged; remaining eligible-frame count correct; Stage 4 timer+keyboard IRQ output continues after Stage 5D; no regressions. |
| Stage 6C - PMM minimal free API and pending-free tracking | complete | 2026-04-01 | Export stage6c_pmm_free_frame and stage6c_pmm_get_pending_free_frames; validate free inputs and record deterministic pending-free entries only; no reuse activation and no allocation-path behavior change | QEMU validation: build and boot succeeded; Stage 5A/5B/5C output intact; Stage 5D deterministic allocation output intact; allocated frame addresses unchanged; remaining eligible-frame count correct; Stage 4 timer+keyboard IRQ output continued after Stage 5D; no regressions. |
| Stage 6D - Deterministic pending-free reuse activation and validation | complete | 2026-04-01 | Activate FIFO reuse from pending-free frames before fresh allocation, add minimal already-issued free check, and preserve baseline Stage 5D/6B behavior when reuse is not exercised | QEMU validation: dedicated Stage 6D reuse self-test passed with PASS markers for alloc A/B, free A accept, free B accept, duplicate free reject, never-issued reject, pending count after free, fifo reuse order, pending drained, and overall; baseline Stage 5A/5B/5C and Stage 5D outputs remained intact with no regressions. |
| Stage 6 - PMM lifecycle baseline (6A-6D) | complete | 2026-04-01 | Stage 6A through Stage 6D delivered explicit PMM state, minimal alloc/free APIs, deterministic pending-free tracking, FIFO reuse activation, and validated reuse self-test behavior | Stage 6A, Stage 6B, Stage 6C, and Stage 6D evidence set |
| Stage 7A - Paging model and non-activating groundwork | complete | 2026-04-02 | Define 32-bit non-PAE 4 KiB paging constants/flags/helpers, identity-map model, and deterministic self-check output; no CR0/CR3 activation | QEMU validation: Stage 7A self-check PASS for 0x12345000 decomposition and masked entry output; Stage 6 runtime unchanged |
| Stage 7B - Static paging structures and setup groundwork | not-started | TBD | Define static paging structures and initial mapping setup without activation | Pending Stage 7B |
| Stage 7C - Paging activation path | not-started | TBD | Introduce controlled CR3/CR0 paging enable path | Pending Stage 7C |
| Stage 7D - Identity-mapping validation and fault-aware verification | not-started | TBD | Validate identity mapping correctness and paging fault behavior | Pending Stage 7D |
| Stage 7 - Paging bring-up suite (7A-7D) | in-progress | TBD | Complete paging model, setup, activation, and validation with no regressions | Stage 7A complete |

## Current focus

- Stage 5A through Stage 5D are complete and verified.
- Full Stage 5 is complete.
- Stage 6A through Stage 6D are complete and validated.
- Full Stage 6 is complete.
- Stage 7A is complete (non-activating paging groundwork).
- Next focus: Stage 7B planning and scope definition (still no paging activation).

## Weekly update template

- Week of: 2026-04-02

- Completed:
  - Stage 5A memory map parsing
  - Stage 5B memory accounting and policy filtering
  - Stage 5C physical frame bookkeeping groundwork
  - Stage 5D deterministic frame allocation boot test (validated)
  - Stage 6A PMM internal cleanup completed and validated (behavior preserved, no regressions)
  - Stage 6B PMM public API surface completed and validated (behavior preserved, no regressions)
  - Stage 6C minimal PMM free API and deterministic pending-free tracking completed and validated (no reuse activation, no regressions)
  - Stage 6D deterministic FIFO reuse activation completed and validated (baseline path stable, no regressions)
  - Stage 6D dedicated reuse self-test passed in QEMU (accept/reject/FIFO checks all PASS)
  - Stage 6 aggregate completion achieved (6A through 6D)
  - Stage 7A non-activating paging groundwork completed (helpers/constants/flags/decomposition + masked entry self-check)

- Blockers:
  - None

- Next focus:
  - Stage 7B planning: define next paging milestone boundaries while keeping paging disabled
  - Define validation gates for eventual page-structure construction without CR0/CR3 writes
  - Keep allocator and Stage 6 runtime behavior stable during Stage 7 follow-on work

- Risk changes:
  - Stage 5D allocator path is intentionally minimal and non-freeing; full allocator lifecycle remains future work
  - Stage 6A kept allocator behavior unchanged by design and validated against existing Stage 5D and Stage 4 markers
  - Stage 6B validation completed with no regressions
  - Stage 6C validation completed with no regressions
  - Stage 6D validation completed with no regressions; reuse activation remains minimal and deterministic
