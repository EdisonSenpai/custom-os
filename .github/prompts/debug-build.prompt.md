---
description: "Diagnose and fix custom OS build, link, image, or boot failures with minimal risk and clear validation."
name: "Debug OS Build"
argument-hint: "Paste failing command, error output, and expected behavior"
agent: "builder"
---

Diagnose and fix the user-provided custom operating system build or boot issue.

Follow this workflow:
1. Reproduce or reason from the provided command output and failure details.
2. If required details are missing, invoke the interviewer agent to ask concise multiple-choice clarifying questions with free-form answers enabled.
3. Classify the failure stage: compile, link, image generation, boot entry, mode transition, or kernel handoff.
4. Inspect the relevant build scripts, linker scripts, startup assembly, and kernel interfaces.
5. Propose a minimal-risk fix plan with explicit ABI and memory-layout impact notes.
6. Ask for approval before large architectural changes.
7. Apply minimal edits, then run available validation commands.
8. Report root cause, files changed, validation outcomes, and any residual risks.
