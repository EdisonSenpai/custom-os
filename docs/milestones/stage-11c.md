# Stage 11C — Kobject List Traversal and Lookup

Status: complete

## Overview

Stage 11C adds deterministic traversal and lookup over the Stage 11B `kobject_list` so callers can query objects by id and read list size without changing list structure.

Execution order remains:

- Stage 11A self-check
- Stage 11B self-check
- Stage 11C self-check
- Stage 6 runtime initialization and IRQ activity

## APIs

- `struct kobject_node* kobject_list_find_by_id(const struct kobject_list* list, uint32_t id);`
  - Traverses from `list->head` through `next` pointers.
  - Returns the first node whose `id` matches.
  - Returns `NULL` when `list` is `NULL` or when no match exists.

- `uint32_t kobject_list_count(const struct kobject_list* list);`
  - Returns `list->count` for a valid list.
  - Returns `0` when `list` is `NULL`.

## Behavior Guarantees

- Deterministic traversal order: head-to-tail following `next`.
- No allocator changes: Stage 11C uses existing Stage 11A/11B object and list behavior.
- No mutation during traversal/lookup: lookup and count are read-only operations.

## Self-Check Coverage

The Stage 11C self-check performs:

- List initialization.
- Node creation and append setup.
- Lookup validation for first node id.
- Lookup validation for second node id.
- Lookup validation for missing id.
- Count validation (`2`).
- Cleanup validation by destroying allocated nodes.

## Expected Output Markers

- `custom-os Stage 11C list traversal validation begin`
- `custom-os Stage 11C find first result: PASS|FAIL`
- `custom-os Stage 11C find second result: PASS|FAIL`
- `custom-os Stage 11C find missing result: PASS|FAIL`
- `custom-os Stage 11C count result: PASS|FAIL`
- `custom-os Stage 11C cleanup result: PASS|FAIL`
- `custom-os Stage 11C: PASS|FAIL`

## PASS Condition

Stage 11C passes only when all Stage 11C checks report PASS and the final marker is exactly:

- `custom-os Stage 11C: PASS`
