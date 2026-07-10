# TDD Workflow

This project follows Test-Driven Development. When implementing a new feature or component:

1. **Tests first** — The qa-tester agent writes failing tests (unit and/or property-based) that define the expected behaviour BEFORE any production code is written.
2. **Minimal implementation** — The developer agent then writes the minimum production code needed to make those tests pass.
3. **Verify** — Run the tests to confirm they pass. If they don't, the developer fixes the implementation until they do.

## Task Ordering in Specs

When generating tasks.md for a new feature, test tasks should appear BEFORE their corresponding implementation tasks in the dependency graph (or at minimum in the same wave). The pattern is:

- Write test for behaviour X (expected to fail — no implementation yet)
- Implement behaviour X (tests now pass)

## Agent Responsibilities

- **qa-tester**: Writes tests based on requirements and design docs. Tests should compile (may need stub headers/forward declarations) but are expected to FAIL until the developer implements the feature.
- **developer**: Implements production code that makes the qa-tester's tests pass. Must not modify test assertions to make them pass — only production code.

## Practical Notes

- Tests go in `Tests/` directory using Catch2 v3 and RapidCheck.
- New test `.cpp` files must be added to `Tests/40kRL_Tests.vcxproj`.
- New source `.cpp` files must be added to BOTH `40kRL.vcxproj` AND `Tests/40kRL_Tests.vcxproj`.
- Property-based tests use `rc::check` with minimum 100 iterations.
