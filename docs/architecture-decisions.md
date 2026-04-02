# Architecture Decisions (Stage 0)

Purpose: record early decisions before implementation to reduce churn and ABI risk.

## Decision log

1. Target architecture
- Decision: x86_64 only for initial bring-up.
- Status: accepted.
- Rationale: keeps debugging and linker/boot assumptions focused.
- Stage 0 note: boot entry ABI is temporarily i386-compatible via Multiboot2 to simplify first boot pipeline validation. Long-mode transition remains a Stage 1 task.

2. Boot path
- Decision: GRUB + Multiboot2.
- Status: accepted.
- Rationale: reduce early bootloader complexity and focus on kernel fundamentals.

3. Kernel model
- Decision: monolithic kernel.
- Status: accepted.
- Rationale: simpler integration for solo development.

4. Language split
- Decision: C-first, assembly only for unavoidable boundaries.
- Status: accepted.
- Rationale: predictable low-level behavior with manageable complexity.

5. First execution environment
- Decision: QEMU-first.
- Status: accepted.
- Rationale: repeatable testing and faster iteration.

## Open decisions (defer)

- Initial physical memory manager strategy.
- Initial virtual memory mapping layout details.
- Syscall entry mechanism and ABI versioning policy.

## Change policy

- Update this file before changing architecture-level assumptions.
- If a decision affects boot, linker, or ABI boundaries, add migration notes.
