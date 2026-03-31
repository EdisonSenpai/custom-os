---
description: "Implement a new custom operating system feature with safe planning, minimal edits, and validation."
name: "New OS Feature"
argument-hint: "Describe the feature, target architecture, constraints, and success criteria"
agent: "builder"
---

Implement the user-provided custom operating system feature request.

Follow this workflow:
1. Inspect repository context and relevant boot, kernel, linker, and build files before editing.
2. If requirements are ambiguous, invoke the interviewer agent to ask concise multiple-choice questions with free-form answers enabled.
3. Propose an implementation plan that includes touched files, memory-layout or ABI impact, and validation steps.
4. Ask for approval before large architectural changes.
5. Prefer C and C++ for kernel-related implementation unless the user requests otherwise.
6. Apply minimal, controlled edits and avoid unrelated refactors.
7. Run available build, test, and emulator checks.
8. Report what changed, why it changed, validation results, and remaining risks.
