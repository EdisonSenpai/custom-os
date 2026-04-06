# CustomOS (working title: AnimeOS)

> Current status: Stage 11 complete (11A + 11B + 11C + 11D + 11E) and verified in QEMU. Stage 12 not started.

CustomOS is a from-scratch operating system project focused on low-level correctness, inspectability, and disciplined incremental bring-up. The long-term direction is an anime-themed experimental OS identity built on top of a technically rigorous kernel foundation. The current baseline is Stage 11 complete with active paging plus validated object lifecycle behavior over the Stage 10 public allocation API baseline.

## Architecture snapshot (Stage 11)

- 32-bit protected mode (Multiboot2 entry)
- Paging enabled (CR3 loaded and CR0.PG set)
- Static first-4 MiB identity mapping active
- Bootloader-provided GDT in use
- IDT installed at runtime
- Exceptions handled via assembly stubs to C dispatcher
- PIC remapped for hardware IRQ vectors
- PIT configured for periodic IRQ0 timer interrupts
- Keyboard IRQ1 routed through IDT and raw scancodes read from port 0x60
- Multiboot2 memory map parsing and early frame bookkeeping
- Multiboot2 header is emitted from `stage0_entry.S` in an allocatable section (`.section .multiboot, "a"`)
- Minimal PMM API: deterministic frame allocation, minimal free API, deterministic pending-free tracking, and FIFO reuse activation
- PMM reservation filtering excludes kernel-occupied physical memory via linker-exported `__kernel_phys_start` and `__kernel_phys_end`
- PMM lifecycle baseline (Stage 6A through Stage 6D) remains active
- Stage 8A virtual layout policy baseline remains active
- Stage 8B minimal single-page VMM map/unmap/query interface remains active
- Stage 8C minimal bump-style heap bootstrap remains active
- Stage 8D validation checks confirm controlled allocation behavior without allocator redesign
- Stage 9A/9B/9C/9D heap lifecycle validation baseline remains active
- Stage 10A/10B/10C/10D/10E public allocation API validation baseline remains active
- Stage 11A/11B/11C/11D/11E object lifecycle validation baseline remains active
- Stage 6 timer and keyboard runtime behavior remains active under paging

## Current baseline (Stage 11)

Implemented and verified:

- Build pipeline via Make.
- Freestanding kernel ELF target.
- GRUB + Multiboot2 bootable ISO generation.
- QEMU run scripts for WSL/Linux and Windows PowerShell.
- Deterministic early init sequence.
- Multiboot2 entry validation.
- Minimal panic path for fatal early boot errors.
- IDT setup.
- Basic CPU exception handling for early bring-up.
- Exception diagnostics output to VGA text mode and COM1 serial.
- PIC remap and IRQ0/IRQ1 operation with EOI handling.
- PIT periodic tick configuration.
- Recurring Stage 6 tick output on COM1 serial while idle.
- Raw keyboard scancode logging on COM1 serial.
- Stage 5A: Multiboot2 memory map parsing.
- Stage 5B: raw and policy-usable RAM accounting summary.
- Stage 5C: frame-range bookkeeping groundwork.
- Stage 5D: deterministic early-boot frame allocation smoke test.
- Stage 6A: PMM state formalization and internal cleanup.
- Stage 6B: minimal exported PMM allocation API.
- Stage 6C: minimal exported PMM free API and deterministic pending-free tracking.
- Stage 6D: deterministic FIFO reuse activation and dedicated reuse self-test validation.
- Stage 7A: non-activating paging model helpers and deterministic groundwork self-check.
- Stage 7B: static aligned paging structures and deterministic first-4 MiB identity-map setup.
- Stage 7C: explicit CR3 load and CR0.PG paging activation path.
- Stage 7D: active paging validation with deterministic CR3/CR0 markers, identity probes, and non-destructive page-fault-awareness confirmation.
- Stage 8A: explicit virtual memory layout policy constants and address classification helpers.
- Stage 8B: minimal single-page VMM query/resolve/map/unmap interface.
- Stage 8C: minimal PMM-backed, VMM-mapped bump-style kernel heap bootstrap.
- Stage 8D: controlled allocation validation for alignment/order, mapped-end growth, and heap-window bounds.
- Stage 9A: heap free groundwork and allocation tracking.
- Stage 9B: deterministic exact-size freed-block reuse activation.
- Stage 9C: deterministic split-capable reuse with valid leftover fragment handling.
- Stage 9D: lifecycle validation suite for exact reuse, split reuse, leftover reuse, invalid free rejection, and double free rejection.
- Stage 10A: public `kmalloc`/`kfree` API wrapper validation.
- Stage 10B: wrapper semantics and invariants validation (`kmalloc(0)`, `kfree(NULL)`).
- Stage 10C: memory utility primitive validation (`kmemset`, `kmemcpy`, `kmemmove`, `kmemcmp`).
- Stage 10D: public API lifecycle hardening validation.
- Stage 10E: final composed allocation API validation suite.
- Stage 11A: dynamic kernel object groundwork (`kobject_node` create/destroy).
- Stage 11B: linked structure construction (`kobject_list` init/append).
- Stage 11C: traversal and lookup (`kobject_list_find_by_id`, `kobject_list_count`).
- Stage 11D: removal and cleanup (`kobject_list_remove_by_id`).
- Stage 11E: composed object lifecycle validation suite.

## Stage 10 and Stage 11 highlights

- Stage 4 interrupt runtime behavior remains active after Stage 6 PMM lifecycle bring-up.
- Stage 5A and 5B provide safe parsing and accounting summaries from Multiboot2 data.
- Stage 5C establishes allocator-ready frame range bookkeeping.
- Stage 5D proves deterministic frame handout behavior without introducing full allocator lifecycle complexity.
- Stage 6C adds deterministic pending-free tracking.
- Stage 6D activates deterministic FIFO reuse while preserving baseline boot/runtime behavior.
- Stage 7C enables paging with explicit CR3/CR0 control-register operations.
- Stage 7D validates active paging state and first-4 MiB identity behavior while preserving Stage 6 timer/keyboard runtime.
- Stage 8D finalizes Stage 8 with deterministic heap growth and bounds validation under active paging.
- Stage 9D finalizes Stage 9 with deterministic heap lifecycle validation over Stage 9A/9B/9C behavior.
- Stage 10A through Stage 10E finalize the public allocation API surface with deterministic composed validation and no allocator redesign.
- Stage 11A through Stage 11E finalize deterministic object lifecycle behavior over Stage 10 public API primitives.

## Scope boundaries

Implemented now:

- Reproducible build and run flow from source to bootable ISO.
- Stage 6 deterministic startup messages on VGA and serial.
- Stage 4 IRQ runtime behavior (timer plus keyboard) and Stage 2 exception diagnostics.
- Minimal PMM lifecycle through Stage 6A to Stage 6D.
- Static identity-mapped paging baseline through Stage 7A to Stage 7D.
- Minimal VMM policy/mapping and controlled heap bootstrap through Stage 8A to Stage 8D.
- Heap lifecycle validation baseline through Stage 9A to Stage 9D.
- Public allocation API validation suite through Stage 10A to Stage 10E.
- Object lifecycle validation suite through Stage 11A to Stage 11E.

Deliberately not implemented yet:

- Advanced frame lifecycle features beyond current deterministic minimal PMM model.
- Advanced virtual memory work beyond the current static first-4 MiB identity-mapped paging baseline.
- Advanced heap allocator capabilities beyond current Stage 11 validation baseline (coalescing policy expansion, advanced fit strategies, realloc or calloc, slab or buddy design).
- Advanced object management features beyond current Stage 11 validation baseline (reference counting, richer container models, policy layers).
- Scheduler and task switching.
- Filesystem and user-mode runtime.
- Full x86_64 long-mode runtime.

## Repository map

- docs/: architecture decisions, boot assumptions, milestones, and toolchain notes.
- boot/grub/: GRUB configuration for bootable image generation.
- arch/x86_64/: architecture-specific entry and interrupt/exception assembly stubs.
- linker/: linker script and memory layout contract.
- kernel/init/: staged early init and self-check flow through Stage 11 diagnostics.
- build/: shared Make configuration and build targets.
- scripts/: QEMU run wrappers for shell and PowerShell workflows.

## Build commands

### WSL/Linux

```sh
make help
make iso
make run
```

### Windows PowerShell

```powershell
make iso
.\scripts\run-qemu.ps1 -IsoPath out\custom-os.iso
```

## Expected output

### Normal Stage 11 run

VGA and COM1 serial should show:

- custom-os v0.11.5 (Stage 11E): init start
- custom-os Stage 5A/B/C/D summary markers
- custom-os Stage 7A/7B/7C/7D markers
- custom-os Stage 8A/8B/8C/8D markers
- custom-os Stage 9A/9B/9C/9D markers
- custom-os Stage 10A/10B/10C/10D/10E markers
- custom-os Stage 11A/11B/11C/11D/11E markers
- custom-os Stage 6: Multiboot2 handoff OK
- custom-os Stage 6: IDT installed
- custom-os Stage 6: PIC remapped + PIT started
- custom-os Stage 6: deterministic init OK
- recurring serial lines like custom-os Stage 6 tick: 0x00000064
- keyboard serial lines like custom-os Stage 6 scancode: 0x0000001E

### Stage 6D dedicated reuse self-test

Note: The current build system does not yet wire that flag through and the temporary source define was used for validation. The self-test is enabled temporarily by setting STAGE6D_FORCE_REUSE_TEST in kernel/init/stage0_main.c:

```sh
make clean
make run STAGE6D_FORCE_REUSE_TEST=1
```

Expected serial markers include:

- custom-os Stage 6D test alloc A/B: PASS
- custom-os Stage 6D test free A accept: PASS
- custom-os Stage 6D test free B accept: PASS
- custom-os Stage 6D test duplicate free reject: PASS
- custom-os Stage 6D test never-issued reject: PASS
- custom-os Stage 6D test pending count after free: PASS
- custom-os Stage 6D test fifo reuse order: PASS
- custom-os Stage 6D test pending drained: PASS
- custom-os Stage 6D test overall: PASS

### Forced INT3 exception test

Note: QEMU may appear paused after an exception. This is expected because the CPU halts (cli + hlt).

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

Expected exception-path markers:

- custom-os Stage 6: triggering INT3 test
- custom-os Stage 2 EXCEPTION
- #BP breakpoint
- vector: 0x00000003
- error : 0x00000000
- eip   : 0x????????
- cs    : 0x????????
- eflags: 0x????????

## Panic-path self-test (Stage 1 regression check)

Keep this test available as a regression guard.

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

Expected panic output (VGA and serial):

- custom-os Stage 6 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Future direction

- Keep the Stage 11 complete baseline stable under routine validation.
- Preserve Stage 6 runtime behavior while paging is active.
- Stage 12 is not started.

## Key docs

- docs/architecture-decisions.md
- docs/boot-handoff-notes.md
- docs/toolchain-notes.md
- docs/milestone-tracker.md
- docs/milestones/stage-0.md
- docs/milestones/stage-1.md
- docs/milestones/stage-2.md
- docs/milestones/stage-3.md
- docs/milestones/stage-4.md
- docs/milestones/stage-5a.md
- docs/milestones/stage-5b.md
- docs/milestones/stage-5c.md
- docs/milestones/stage-5d.md
- docs/milestones/stage-6a.md
- docs/milestones/stage-6b.md
- docs/milestones/stage-6c.md
- docs/milestones/stage-6d.md
- docs/milestones/stage-7a.md
- docs/milestones/stage-7b.md
- docs/milestones/stage-7c.md
- docs/milestones/stage-7d.md
- docs/milestones/stage-8a.md
- docs/milestones/stage-8b.md
- docs/milestones/stage-8c.md
- docs/milestones/stage-8d.md
- docs/milestones/stage-9a.md
- docs/milestones/stage-9b.md
- docs/milestones/stage-9c.md
- docs/milestones/stage-9d.md
- docs/milestones/stage-10a.md
- docs/milestones/stage-10b.md
- docs/milestones/stage-10c.md
- docs/milestones/stage-10d.md
- docs/milestones/stage-10e.md
- docs/milestones/stage-11a.md
- docs/milestones/stage-11b.md
- docs/milestones/stage-11c.md
- docs/milestones/stage-11d.md
- docs/milestones/stage-11e.md
