# Requirements Document

## Introduction

The test binary (40kRL_Tests.exe) hangs on GitHub Actions CI runners because the `Engine` constructor calls `TCODConsole::initRoot()` and `SDL_StartTextInput()` during global initialization, and the headless Windows runner (SDL_VIDEODRIVER=dummy) cannot satisfy these calls. This spec defines the requirements for refactoring the test infrastructure so that all tests run reliably within the CI timeout on a headless runner, while preserving local test behaviour.

## Glossary

- **Test_Binary**: The compiled executable `40kRL_Tests.exe` produced by `Tests/40kRL_Tests.vcxproj`
- **CI_Runner**: The GitHub Actions `windows-latest` headless runner with `SDL_VIDEODRIVER=dummy` and `SDL_AUDIODRIVER=dummy` environment variables set
- **Engine**: The global `Engine` instance declared in `catch_main.cpp` that owns game state (actor list, map, camera, GUI)
- **Pure_Logic_Test**: A test that exercises game logic (damage formulas, data structures, formatting, dice rolls, hit locations) without requiring a TCODConsole window or SDL context
- **Integration_Test**: A test that requires the full Engine (player, map, actors, GUI) and therefore depends on libtcod/SDL window creation
- **Headless_Mode**: An execution context where no real display is available; detected via environment variable or preprocessor define
- **Catch2_Tag**: A string tag (e.g. `[integration]`, `[pbt]`) applied to a Catch2 TEST_CASE that allows selective inclusion or exclusion at runtime

## Requirements

### Requirement 1: CI Test Completion

**User Story:** As a developer, I want the CI test step to complete reliably, so that pull requests receive timely pass/fail feedback without manual intervention.

#### Acceptance Criteria

1. WHEN the Test_Binary is executed on the CI_Runner, THE Test_Binary SHALL complete execution and return an exit code within 5 minutes
2. WHEN the Test_Binary is executed on the CI_Runner, THE Test_Binary SHALL NOT call `TCODConsole::initRoot()` or `SDL_StartTextInput()` for Pure_Logic_Tests
3. IF the Test_Binary process exceeds the 5-minute timeout on the CI_Runner, THEN THE CI workflow SHALL report the test step as failed with a non-zero exit code

### Requirement 2: Pure Logic Test Independence

**User Story:** As a developer, I want tests that verify game logic to run without a window or SDL context, so that they work on headless CI without modification.

#### Acceptance Criteria

1. THE Pure_Logic_Test suite SHALL execute without requiring a TCODConsole window, SDL video driver, or GPU context
2. WHEN a Pure_Logic_Test is executed on the CI_Runner, THE Test_Binary SHALL produce the same pass/fail result as when executed locally with a display
3. THE Pure_Logic_Test suite SHALL NOT depend on the global `Engine` instance being fully initialized (constructor + `init()` called)
4. WHEN a new Pure_Logic_Test file is added to the Tests/ directory, THE test author SHALL be able to run it on CI without additional build or configuration changes beyond adding it to the vcxproj

### Requirement 3: Integration Test Availability

**User Story:** As a developer, I want integration tests that exercise Engine-dependent game state (movement, combat, doors, map generation) to remain compilable and runnable, so that full-stack testing is preserved.

#### Acceptance Criteria

1. THE Integration_Test suite SHALL continue to compile and link against the Engine and libtcod
2. WHEN Integration_Tests are executed locally with a real display, THE Test_Binary SHALL produce the same pass/fail results as before this refactoring
3. WHILE the CI_Runner does not support window creation, THE CI workflow SHALL exclude Integration_Tests using Catch2_Tag filtering (e.g. `~[integration]`)
4. THE Integration_Tests SHALL be tagged with the `[integration]` Catch2_Tag so that the CI workflow can selectively exclude them

### Requirement 4: No Blocking Calls in Test Code

**User Story:** As a developer, I want the test binary to never block on interactive functions, so that CI hangs are eliminated.

#### Acceptance Criteria

1. THE Test_Binary SHALL NOT call `Menu::pick()`, `TCODConsole::flush()`, `TCODConsole::waitForKeypress()`, or any function that blocks waiting for user input or display sync
2. WHEN a test requires simulated input, THE test SHALL inject input state programmatically rather than waiting for keyboard events
3. IF a blocking call is inadvertently introduced into a test, THEN THE CI_Runner SHALL detect the hang via the 5-minute timeout and report failure

### Requirement 5: Conditional Engine Initialization

**User Story:** As a developer, I want the Engine initialization to detect headless mode and skip window creation, so that integration tests can still run on CI in the future if SDL dummy mode is fixed.

#### Acceptance Criteria

1. WHEN the environment variable `KIRL_HEADLESS` is set to `1`, THE Engine constructor SHALL skip calls to `TCODConsole::setCustomFont()`, `TCODConsole::initRoot()`, and `SDL_StartTextInput()`
2. WHEN the environment variable `KIRL_HEADLESS` is set to `1`, THE Engine constructor SHALL still initialize non-graphical members (gameStatus, player pointer, dungeon level, fovRadius)
3. WHEN `KIRL_HEADLESS` is not set or is set to `0`, THE Engine constructor SHALL perform full initialization including window creation (preserving current local behaviour)
4. WHILE `KIRL_HEADLESS` is set to `1`, THE `engine.init()` function SHALL load Lua scripts and create game objects but SHALL skip any rendering-dependent initialization

### Requirement 6: Existing Test Preservation

**User Story:** As a developer, I want all existing tests to continue passing after the refactoring, so that no functionality is regressed.

#### Acceptance Criteria

1. THE refactored Test_Binary SHALL pass all currently-passing test cases when run locally with a real display
2. WHEN a test case passes before refactoring, THE same test case SHALL pass after refactoring without modification to its assertions
3. THE refactoring SHALL NOT require rewriting test assertions, test logic, or test data for currently-passing tests
4. IF a test must be re-tagged or moved to a different source file, THEN THE test logic and assertions SHALL remain unchanged

### Requirement 7: Minimal Code Impact

**User Story:** As a developer, I want the refactoring to minimize changes to production game code, so that risk of introducing bugs in the game itself is low.

#### Acceptance Criteria

1. THE refactoring SHALL NOT modify the game's rendering pipeline, game loop, or gameplay logic in production builds
2. THE refactoring SHALL limit production code changes to the Engine constructor and `Engine::init()` method (headless guard only)
3. WHEN the game is built and run normally (without `KIRL_HEADLESS`), THE game SHALL behave identically to before the refactoring
4. THE refactoring SHALL NOT introduce new third-party dependencies beyond what is already in vcpkg.json

### Requirement 8: Build System Consistency

**User Story:** As a developer, I want both vcxproj files to remain in sync, so that the test binary links correctly against all game source files.

#### Acceptance Criteria

1. THE `Tests/40kRL_Tests.vcxproj` SHALL continue to include all game source files except `main.cpp`
2. THE `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj` SHALL remain synchronized for any new or removed source files resulting from this refactoring
3. THE Test_Binary SHALL link successfully in both Debug and Release configurations on x64
4. THE CI workflow SHALL build and test both Debug and Release configurations

### Requirement 9: Lua Script Accessibility

**User Story:** As a developer, I want Lua scripts to remain accessible at test runtime, so that tests exercising Lua-loaded data (equipment, enemies, careers) continue to function.

#### Acceptance Criteria

1. WHEN `engine.init()` is called in Headless_Mode, THE Engine SHALL load Lua scripts from the `Scripts/` directory
2. THE working directory for the Test_Binary on the CI_Runner SHALL contain the `Scripts/` folder or a path SHALL be configured so that Lua `dofile()` calls resolve correctly
3. IF a Lua script fails to load during test initialization, THEN THE Engine SHALL report the error and terminate rather than silently continuing with missing data
