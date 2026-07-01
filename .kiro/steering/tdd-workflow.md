---
inclusion: auto
---

# TDD Workflow

When generating tasks for any spec, follow test-driven development ordering:

1. **Test first** — Write the test (unit test or property-based test) that defines the expected behavior. The test should fail because the feature doesn't exist yet.
2. **Implement** — Write the minimum code to make the test pass.
3. **Commit** — Stage and commit after each test+implementation pair with a descriptive message.
4. **Refactor** — Clean up if needed, ensure tests still pass, commit again.

## Task Ordering Convention

For each feature unit in a task list:
- The test sub-task comes BEFORE the implementation sub-task
- Test tasks are NOT optional — they are required
- Property-based tests use RapidCheck (in `Tests/lib/rapidcheck_catch.h`)
- Unit tests use Catch2 (in `Tests/lib/catch_amalgamated.hpp`)

## Test File Location

- All tests go in the `Tests/` directory
- Name pattern: `test_<feature>.cpp` (e.g., `test_outdoor_map.cpp`)
- Include the test in the `Tests/40kRL_Tests.vcxproj` project

## Commit Pattern

Each commit should contain either:
- A failing test (red) — message: "Test: [description] (red)"
- The implementation that makes it pass (green) — message: "[Feature]: [description]"

Or combined if the test+impl are small enough:
- "TDD: [description] — test + implementation"
