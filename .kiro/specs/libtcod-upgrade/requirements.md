# Requirements Document

## Introduction

This specification covers upgrading the 40kRL project from libtcod 1.15.0 (locally bundled with SDL2) to libtcod 2.2.x (the latest C++ release, which uses SDL3 instead of SDL2). The upgrade resolves all breaking API changes, replaces deprecated rendering calls, fixes template and access-specifier issues exposed by modern MSVC, ensures RapidCheck is available for property-based testing in CI, and re-enables the GitHub Actions CI workflow so that builds and tests run automatically on push.

libtcod 2.x retains the legacy C++ wrapper classes (TCODConsole, TCODMap, TCODBsp, TCODRandom, TCODNoise, TCODZip) in a deprecated-but-compilable state. The project will suppress C4996 deprecation warnings initially and migrate to newer APIs incrementally in future work.

## Glossary

- **Build_System**: The Visual Studio solution (40kRL.sln) comprising the game project (40kRL.vcxproj) and the test project (40kRL_Tests.vcxproj), compiled with MSVC v142+ using C++17.
- **CI_Pipeline**: The GitHub Actions workflow defined in `.github/workflows/ci.yml` that builds and tests the project on `windows-latest` using vcpkg-provided dependencies.
- **vcpkg**: Microsoft's C++ package manager, used by the CI_Pipeline to install libtcod, sol2, and Lua.
- **libtcod**: The roguelike toolkit library (version 2.2.x) providing console rendering, FOV computation, BSP generation, noise generation, random number generation, and serialization (TCODZip). Uses SDL3 as its backend.
- **SDL3**: The Simple DirectMedia Layer version 3, the runtime dependency for libtcod 2.x (replacing SDL2 used by libtcod 1.x).
- **Deprecated_Character_Constants**: The set of `TCOD_CHAR_*` enumerators removed in libtcod 1.16+: TCOD_CHAR_SPADE, TCOD_CHAR_DCROSS, TCOD_CHAR_DTEES, TCOD_CHAR_DTEEN, TCOD_CHAR_DTEEE, TCOD_CHAR_DTEEW, TCOD_CHAR_DNE, TCOD_CHAR_DNW, TCOD_CHAR_DSE, TCOD_CHAR_DSW, TCOD_CHAR_DVLINE, TCOD_CHAR_DHLINE, TCOD_CHAR_RADIO_UNSET.
- **Deprecated_Rendering_Functions**: The per-cell rendering functions deprecated in libtcod 2.1: TCOD_console_set_char, TCOD_console_set_char_background, TCOD_console_set_char_foreground, TCOD_console_put_char, TCOD_console_put_char_ex (and their C++ wrappers setChar, setCharBackground, setCharForeground, putChar, putCharEx). Replaced by TCOD_console_put_rgb and TCOD_printf_rgb.
- **Deprecated_Event_API**: The legacy event functions deprecated in libtcod 2.2: TCODSystem::checkForEvent, TCOD_EVENT_KEY_PRESS, TCOD_key_t, TCOD_mouse_t. Replaced by SDL3 event polling.
- **Gui_Message_Template**: The variadic template method `Gui::message` that formats and logs coloured messages to the HUD.
- **Actor_Glyph_Field**: The private `int glyph` member of the Actor class, accessed externally in `Engine::nextLevel`.
- **RapidCheck**: A C++ property-based testing library used alongside Catch2 in the test project.
- **Test_Project**: The 40kRL_Tests.vcxproj project that compiles and links all game source files (except main.cpp) with Catch2 and RapidCheck test files.
- **FetchContent**: CMake's module for downloading and building dependencies at configure time, used when a vcpkg port is unavailable or outdated.
- **Overlay_Port**: A custom vcpkg port definition stored in the project repository that allows vcpkg to build a library version not yet in the official registry.

## Requirements

### Requirement 1: Acquire libtcod 2.2 via vcpkg or FetchContent

**User Story:** As a developer, I want the project to consume libtcod 2.2.x from a package manager or build system integration, so that CI and local builds use the same version without manually bundled binaries.

#### Acceptance Criteria

1. THE Build_System SHALL resolve libtcod headers and libraries from a vcpkg installation, a custom Overlay_Port, or a CMake FetchContent integration for all configurations (Debug and Release) on the x64 platform.
2. THE Build_System SHALL remove references to locally bundled libtcod.lib, libtcod.dll, and SDL2.dll from the solution directory as link-time and runtime dependencies.
3. WHEN vcpkg integration is active, THE Build_System SHALL link against the vcpkg-provided libtcod without requiring hardcoded absolute paths in the project files.
4. THE Build_System SHALL compile successfully against libtcod version 2.2.0 or later.
5. IF the libtcod 2.2 port is not available in the official vcpkg registry, THEN THE Build_System SHALL provide a custom Overlay_Port or FetchContent configuration that builds libtcod 2.2 from source.

### Requirement 2: Provide SDL3 Runtime Dependency

**User Story:** As a developer, I want SDL3 to be available as a runtime dependency, so that libtcod 2.x can initialize its rendering backend at application startup.

#### Acceptance Criteria

1. THE Build_System SHALL make SDL3.dll available in the output directory alongside the game executable for the x64 platform.
2. WHEN libtcod 2.2 is installed via vcpkg, THE Build_System SHALL acquire SDL3 transitively through the libtcod port dependencies.
3. THE CI_Pipeline SHALL produce a build output that includes SDL3.dll in the same directory as the game executable.
4. THE Build_System SHALL remove the bundled SDL2.dll from the repository since libtcod 2.x no longer depends on SDL2.

### Requirement 3: Replace Deprecated Character Constants

**User Story:** As a developer, I want all removed `TCOD_CHAR_*` constants replaced with their Unicode codepoint equivalents, so that the code compiles without errors on libtcod 2.2.

#### Acceptance Criteria

1. THE Build_System SHALL compile the `chooseWallGlyph` function in Map.cpp without errors for any Deprecated_Character_Constants.
2. THE Build_System SHALL compile the `renderOutdoor` function in Map.cpp without errors for TCOD_CHAR_SPADE.
3. WHEN a Deprecated_Character_Constant was used in libtcod 1.15.0 code, THE Build_System SHALL use the equivalent integer codepoint value defined in the Unicode Box Drawing block or equivalent CP437 mapping.
4. THE Map SHALL render wall tiles identically to the previous behaviour when using the replacement character values.

### Requirement 4: Fix Actor Glyph Access

**User Story:** As a developer, I want the Actor glyph field to be accessible where external code reads or writes it, so that Engine::nextLevel compiles under strict access checking.

#### Acceptance Criteria

1. THE Build_System SHALL compile Engine.cpp without access-violation errors when reading or assigning `stairs->glyph`.
2. THE Actor class SHALL expose the glyph field through the existing public getter `getGlyph()` and setter `setGlyph(int)` methods.
3. WHEN external code previously accessed `Actor::glyph` directly, THE Build_System SHALL use the getter or setter instead.
4. THE Actor class SHALL keep the `glyph` field private to maintain encapsulation.

### Requirement 5: Fix Gui::message Template Signature

**User Story:** As a developer, I want the Gui::message variadic template to compile correctly under modern MSVC, so that all call sites pass arguments without rvalue-reference conversion errors.

#### Acceptance Criteria

1. THE Build_System SHALL compile the Gui_Message_Template without "cannot convert argument from 'std::string' to 'const std::string&&'" errors.
2. THE Gui_Message_Template SHALL accept arguments by forwarding reference (`Args&&...args` with `std::forward`) or by const lvalue reference, following standard C++ parameter pack patterns.
3. WHEN `Gui::message` is called with std::string lvalue arguments, THE Gui_Message_Template SHALL compile and execute without error.
4. WHEN `Gui::message` is called with string literal arguments, THE Gui_Message_Template SHALL compile and execute without error.
5. THE Gui_Message_Template SHALL produce identical formatted output as the previous implementation after the signature fix.

### Requirement 6: Suppress libtcod 2.x Deprecation Warnings

**User Story:** As a developer, I want the project to suppress C4996 deprecation warnings from libtcod 2.x deprecated APIs, so that the build remains warning-clean while using the still-functional legacy C++ wrapper classes.

#### Acceptance Criteria

1. THE Build_System SHALL compile without C4996 warnings originating from usage of deprecated libtcod C++ wrapper classes (TCODConsole, TCODMap, TCODBsp, TCODRandom, TCODNoise, TCODZip).
2. THE Build_System SHALL suppress C4996 warnings only for libtcod-related deprecations, using a targeted pragma or preprocessor definition scoped to libtcod headers.
3. THE Build_System SHALL compile without C4996 warnings from usage of Deprecated_Rendering_Functions (setChar, setCharForeground, setCharBackground).
4. THE Build_System SHALL compile without C4996 warnings from usage of deprecated TCODColor named constants (TCODColor::desaturatedGreen, TCODColor::darkerGreen, TCODColor::lightBlue, etc.).
5. THE Build_System SHALL compile without C4996 warnings from usage of the Deprecated_Event_API (TCODSystem::checkForEvent, TCOD_EVENT_KEY_PRESS).

### Requirement 7: Configure RapidCheck for CI

**User Story:** As a developer, I want RapidCheck available in the CI build environment, so that property-based tests compile and run during CI.

#### Acceptance Criteria

1. THE CI_Pipeline SHALL make the `<rapidcheck.h>` header available to the Test_Project at compile time.
2. THE Test_Project SHALL compile all test files that include `rapidcheck.h` without "cannot find header" errors.
3. THE CI_Pipeline SHALL install or vendor RapidCheck using a method compatible with the existing vcpkg workflow (vcpkg port, git submodule, or header vendoring).
4. WHEN RapidCheck is not available as a vcpkg port, THE Build_System SHALL include RapidCheck as a vendored source dependency in the Tests directory.

### Requirement 8: Fix Test Project Include Paths

**User Story:** As a developer, I want the test project to resolve all game headers in CI, so that the test project compiles when `$(SolutionDir)Headers` resolves correctly.

#### Acceptance Criteria

1. THE Test_Project SHALL resolve `#include "main.h"` and all transitive includes when built from the CI_Pipeline.
2. THE Test_Project SHALL include `$(SolutionDir)Headers` in its include path for both Debug and Release x64 configurations.
3. WHEN the Test_Project is built via `msbuild Tests\40kRL_Tests.vcxproj`, THE Build_System SHALL resolve $(SolutionDir) relative to the .sln file location.
4. THE Test_Project SHALL compile all game source files included in its ItemGroup without missing-header errors.

### Requirement 9: Re-enable CI Workflow

**User Story:** As a developer, I want the GitHub Actions CI workflow to trigger automatically on push and pull request events, so that every change is validated by continuous integration.

#### Acceptance Criteria

1. WHEN a push or pull request targets the main branch, THE CI_Pipeline SHALL trigger automatically.
2. THE CI_Pipeline SHALL build the game project successfully in both Debug and Release configurations for x64.
3. THE CI_Pipeline SHALL build the Test_Project successfully in both Debug and Release configurations for x64.
4. THE CI_Pipeline SHALL execute the test binary and report pass or failure status.
5. IF any build or test step fails, THEN THE CI_Pipeline SHALL report a non-zero exit code and fail the workflow run.
6. THE CI_Pipeline SHALL install libtcod 2.2 (and its transitive SDL3 dependency) using vcpkg with a custom Overlay_Port if the official registry does not yet carry version 2.2.

### Requirement 10: Preserve Existing Functionality

**User Story:** As a developer, I want all existing game behaviour to remain unchanged after the upgrade, so that the libtcod version change is transparent to players.

#### Acceptance Criteria

1. THE Build_System SHALL compile all existing source files without errors after the upgrade.
2. THE Build_System SHALL produce zero new compiler warnings (at Warning Level 3, excluding suppressed C4996 for libtcod deprecations) that did not exist before the upgrade.
3. WHEN the game renders BSP dungeon levels, THE Map SHALL produce the same wall-glyph selection logic as before the upgrade.
4. WHEN the game renders outdoor levels, THE Map SHALL use the same terrain glyphs (spade codepoint for trees, tilde for water, dot for ground) as before the upgrade.
5. THE Engine SHALL maintain the same FOV computation, BSP generation, noise generation, and serialization behaviour as before the upgrade.
6. THE Test_Project SHALL pass all existing tests (test_attacker, test_camera, test_container, test_destructible, test_outdoor_map) after the upgrade.
7. WHEN the game initializes, THE Engine SHALL use the SDL3-backed libtcod renderer (TCOD_renderer_init_sdl3) without requiring code changes beyond the library upgrade.
