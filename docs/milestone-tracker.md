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
| Stage 7B - Static paging structures and setup groundwork | complete | 2026-04-02 | Define aligned static page-directory/page-table aggregates, build deterministic first-4 MiB identity map setup, and validate structure/entry outputs without activation | QEMU validation: Stage 7B self-check PASS for 4 KiB structure sizes, first PTE, last PTE, and PDE[0]; Stage 7A output preserved; no CR0/CR3 writes |
| Stage 7C - Paging activation path | complete | 2026-04-02 | Implement explicit CR3 load plus CR0.PG set activation path using static Stage 7B identity structures with deterministic PASS markers and continued runtime execution | QEMU validation: Stage 7C activation PASS with expected CR3 = observed CR3 = 0x0010B000, observed CR0 = 0x80000011 (CR0.PG set), Stage 7A/7B output preserved, and Stage 6 timer/keyboard runtime unchanged |
| Stage 7D - Identity-mapping validation and fault-aware verification | complete | 2026-04-02 | Validate active paging state, CR3 consistency, first-4 MiB identity probes, and non-destructive page-fault awareness with deterministic serial markers | QEMU validation: Stage 7D markers emitted for validation begin, observed CR3 = 0x0010B000 and observed CR0 = 0x80000011 under active paging, identity probes passed for 0x00001000 and 0x000B8000, page-fault-awareness confirmed via unmapped 0x00400000 with PDE[1] = 0, Stage 7D PASS, and Stage 6 timer/keyboard runtime unchanged |
| Stage 7 - Paging bring-up suite (7A-7D) | complete | 2026-04-02 | Complete paging model, setup, activation, and validation with no regressions | Stage 7A through Stage 7D evidence set |
| Stage 8A - VMM layout policy baseline | complete | 2026-04-02 | Add explicit virtual layout policy constants for active identity first-4 MiB plus future heap/mapping reserved windows, policy-only classification helpers, and deterministic Stage 8A self-check markers | Source integration evidence: new `vmm_layout` policy module wired into kernel build, Stage 8A self-check called immediately after Stage 7D with deterministic PASS/FAIL marker set |
| Stage 8B - Minimal VMM mapping interface | complete | 2026-04-02 | Add minimal single-page VMM query/resolve/map/unmap APIs and deterministic self-check in Stage 8A future mapping reserved window | QEMU validation: Stage 8B markers emitted for mapping interface begin, initial unmapped state PASS, successful map result PASS, resolved physical frame equals expected frame, successful unmap result PASS, final unmapped state PASS, and Stage 8B PASS with Stage 7/8A output preserved |
| Stage 8C - Heap bootstrap allocator groundwork | complete | 2026-04-02 | Add minimal heap bootstrap API over Stage 8A heap reserved window using Stage 6B PMM frames + Stage 8B page mapping with deterministic Stage 8C self-check markers | QEMU validation: Stage 8C markers emitted for heap bootstrap begin, first allocation = 0x00400000, second allocation = 0x00400018, heap current post alloc = 0x00400040, mapped end post alloc = 0x00401000, alignment/ordering check PASS, Stage 8C PASS, and Stage 7/8A/8B output preserved with Stage 6 timer/keyboard runtime unchanged |
| Stage 8 - VMM policy suite (8A-8D) | in-progress | TBD | Stage 8 is split into 8A/8B/8C/8D; 8A and 8B are complete, 8C is in implementation progress, 8D is not started | Stage 8A and Stage 8B evidence present; Stage 8C implementation integrated with validation pending; Stage 8D has no implementation changes yet |

## Current focus

- Stage 7A through Stage 7D remain complete and verified in QEMU.
- Stage 8 is in-progress: Stage 8A and Stage 8B are complete, Stage 8C implementation is in progress, and Stage 8D is not started.
- Current baseline: Stage 7 active first-4 MiB identity-mapped paging plus Stage 8A policy-only layout classification.
- Current focus: Stage 8C post-fix runtime re-validation (heap window mapping gate corrected in Stage 8B VMM policy logic).

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
  - Stage 7B static paging structures and deterministic early identity mapping setup completed (4 KiB-aligned aggregates, first 4 MiB map, PDE[0] wiring, self-check markers)
  - Stage 7C explicit activation path implemented (CR3 load from static Stage 7B directory, CR0.PG set, deterministic pre/post serial markers, post-enable PASS/FAIL check)
  - Stage 7D active paging validation completed (active CR3/CR0 checks, identity probes for 0x00001000 and 0x000B8000, non-destructive page-fault-awareness confirmation via unmapped 0x00400000/PDE[1], deterministic PASS marker)
  - Stage 7 aggregate completion achieved (7A through 7D)
  - Stage 8A VMM layout policy baseline completed (explicit active identity and future reserved windows, policy-only address/region classification helpers, deterministic Stage 8A self-check markers)
  - Stage 8B minimal VMM mapping interface completed (single-page query/resolve/map/unmap APIs in reserved mapping window plus deterministic Stage 8B self-check markers)
  - Stage 8C minimal heap bootstrap implementation integrated (Stage 8A heap reserved window + Stage 6B PMM frame allocation + Stage 8B single-page mapping + deterministic Stage 8C self-check markers)

- Blockers:
  - None

- Next focus:
  - Keep Stage 7 baseline stable under routine boot/runtime checks
  - Preserve Stage 6 timer/keyboard runtime behavior under active paging baseline
  - Validate Stage 8C deterministic markers in QEMU while preserving Stage 7 and Stage 8A/8B outputs
  - Keep Stage 8D not started until Stage 8C validation is complete

- Risk changes:
  - Stage 5D allocator path is intentionally minimal and non-freeing; full allocator lifecycle remains future work
  - Stage 6A kept allocator behavior unchanged by design and validated against existing Stage 5D and Stage 4 markers
  - Stage 6B validation completed with no regressions
  - Stage 6C validation completed with no regressions
  - Stage 6D validation completed with no regressions; reuse activation remains minimal and deterministic
  - Stage 7B completed with paging still disabled by design; no CR3/CR0 writes introduced
  - Stage 7C and Stage 7D now run in early init; preserve first-4 MiB identity assumptions unless milestone scope explicitly changes
  - Stage 8A introduces policy-only virtual layout declarations/classification and deterministic self-check output, with no new mappings, no heap activation, and no paging activation flow changes
  - Stage 8B introduces minimal single-page mapping/query helpers only in the Stage 8A future mapping reserved window, preserving Stage 7 identity mapping behavior and Stage 6 runtime
  - Stage 8C introduces a minimal bump-only heap bootstrap path that maps heap pages on demand from Stage 6B PMM via Stage 8B map-page; no free API, no kmalloc subsystem, and no paging activation changes
