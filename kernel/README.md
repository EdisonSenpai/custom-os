# Kernel Notes

Current status: Stage 9 early kernel baseline is active with paging enabled.

Current implementation location:

- init/stage0_main.c: deterministic bring-up path used from entry to interrupt-driven idle operation.

Current capabilities:

- Early init sequence with Multiboot2 entry validation.
- Panic path output to VGA and COM1 serial.
- IDT install and exception dispatch diagnostics.
- PIC remap and PIT setup for recurring timer IRQ0.
- Keyboard IRQ1 raw scancode capture to COM1 serial.
- Interruptible idle state with periodic timer tick output.
- Stage 5A memory map parsing from Multiboot2 tags.
- Stage 5B RAM accounting summaries (raw and policy-usable).
- Stage 5C frame-range bookkeeping groundwork.
- Stage 5D deterministic boot-time frame allocation smoke test.
- Stage 6B minimal exported PMM allocation API.
- Stage 6C minimal exported PMM free API with deterministic pending-free tracking.
- Stage 6D deterministic FIFO reuse activation with dedicated self-test path.
- Stage 7A paging model and non-activating groundwork.
- Stage 7B static first-4 MiB identity mapping setup.
- Stage 7C explicit CR3 load and CR0.PG activation path.
- Stage 7D active paging validation with identity probes and non-destructive fault-awareness checks.
- Stage 8A virtual memory layout policy baseline.
- Stage 8B minimal single-page VMM query/resolve/map/unmap interface.
- Stage 8C minimal bump-style kernel heap bootstrap backed by PMM and VMM mapping.
- Stage 8D controlled heap-allocation validation (alignment/order, mapped-end growth, heap-window bounds).

Not implemented yet:

- Scheduler and task switching.
- Full memory management subsystem lifecycle.
- Filesystem and user-mode runtime.
- Full keyboard input processing pipeline.
- Advanced virtual memory features beyond the current Stage 8 policy/mapping/heap-bootstrap baseline.
- Full heap allocator lifecycle beyond the current bump allocator validation (free paths, allocator strategy, kmalloc/kfree surface).
