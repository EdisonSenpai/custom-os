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
| Stage 8D - Heap validation and controlled allocation tests | complete | 2026-04-02 | Add dedicated Stage 8D self-check that validates multiple deterministic allocations, monotonic aligned results, mapped-end page growth, and heap-window bounds under the active paging baseline | QEMU validation: Stage 8D markers emitted for allocation validation begin, heap state before test, multiple allocation results, mapped-end advancement PASS, ordering/alignment PASS, heap-window bounds PASS, final Stage 8D PASS, and Stage 6 timer/keyboard runtime unchanged |
| Stage 8 - VMM policy suite (8A-8D) | complete | 2026-04-02 | Stage 8 is split into 8A/8B/8C/8D and all sub-stages are validated with deterministic PASS markers under active paging | Stage 8A through Stage 8D evidence set |
| Stage 9A - Heap free groundwork and allocation tracking | complete | 2026-04-03 | Add allocation metadata and minimal heap free validation API with deterministic valid/invalid/double-free handling, while preserving bump allocation and no reuse | QEMU validation: Stage 9A markers emitted and passed for free A accept, duplicate free reject, invalid free reject, null free reject, no-reuse monotonic allocation check, and final Stage 9A PASS; Stage 7/8 outputs remained intact and Stage 6 timer/keyboard runtime continued |
| Stage 9B - Free-list reuse activation | complete | 2026-04-03 | Activate deterministic reuse path for previously freed exact-size heap blocks before bump growth, while preserving Stage 9A free-validation safety checks | QEMU validation: Stage 9B markers emitted for reuse activation begin, alloc A, alloc B, free A accept PASS, reuse allocation result, reused-address match PASS, and Stage 9B PASS; Stage 9A output remained intact and Stage 6 timer/keyboard runtime continued |
| Stage 9C - Fragmentation-aware block handling | complete | 2026-04-03 | Add minimal deterministic split-capable reuse so larger freed blocks can satisfy smaller aligned allocations while preserving Stage 9A safety and Stage 9B exact-size behavior | QEMU validation: Stage 9C markers emitted for fragmentation-aware reuse begin, alloc large block, free large block accept PASS, alloc smaller block result reusing the same start, reused-start match PASS, leftover fragment allocation, leftover fragment reuse result PASS, and Stage 9C PASS; Stage 7/8/9A/9B outputs remained intact and Stage 6 timer/keyboard runtime continued |
| Stage 9D - Heap lifecycle validation suite | not-started | TBD | Add final validation coverage for allocation/free lifecycle behavior under Stage 9 scope | Not started |
| Stage 9 - Heap lifecycle suite (9A-9D) | in-progress | TBD | Stage 9 is split into 9A/9B/9C/9D; Stage 9A, Stage 9B, and Stage 9C are complete while Stage 9D is not started | Stage 9A, Stage 9B, and Stage 9C evidence sets plus Stage 9D pending |

## Current focus

- Stage 7A through Stage 7D remain complete and verified in QEMU.
- Stage 8A through Stage 8D are complete and verified in QEMU.
- Current baseline: Stage 7 active first-4 MiB identity-mapped paging plus Stage 8A/8B/8C/8D validated policy, mapping, and heap-bootstrap checks.
- Current focus: Stage 9 is split and active. Stage 9A, Stage 9B, and Stage 9C are complete. Stage 9D is not started.

## Weekly update template

- Week of: 2026-04-03

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
  - Stage 8D heap validation and controlled allocation tests completed (multiple deterministic allocations, page-growth/mapped-end advancement, ordering/alignment checks, heap-window bounds checks, Stage 8D PASS)
  - Stage 8 aggregate completion achieved (8A through 8D)
  - Stage 9A heap free groundwork completed and validated in QEMU (valid free accepted, duplicate/invalid/null free rejected, no-reuse monotonic allocation check passed, Stage 9A PASS)
  - Stage 9B deterministic freed-block reuse activation completed and validated in QEMU (free A accepted, exact-size reuse allocation returned freed address, Stage 9B PASS)
  - Stage 9C split-capable reuse implementation completed and validated in QEMU (large allocation succeeded, free large block accept PASS, smaller allocation reused the same start address, leftover fragment allocation succeeded, leftover fragment reuse result PASS, Stage 9C PASS)

- Blockers:
  - None

- Next focus:
  - Keep Stage 7 baseline stable under routine boot/runtime checks
  - Preserve Stage 6 timer/keyboard runtime behavior under active paging baseline
  - Keep Stage 8 baseline stable under routine boot/runtime checks
  - Keep Stage 9A/9B/9C baselines stable; Stage 9D remains not started until explicitly started

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
  - Stage 8D adds validation-only controlled allocation checks over the existing Stage 8C bump allocator; no free logic, no allocator redesign, and no Stage 9 behavior changes
  - Stage 9A adds metadata-backed free validation only; freed blocks are not reused and fragmentation handling is intentionally deferred to Stage 9B/9C
  - Stage 9B activates deterministic exact-size reuse of freed blocks before bump growth; no splitting, no coalescing, and no fragmentation-aware policy yet
  - Stage 9C adds deterministic split-capable reuse for larger freed blocks with minimal fragment validation; no coalescing and no advanced fit strategy yet
