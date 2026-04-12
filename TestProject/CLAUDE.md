# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Layout

This is the **NEXUS Framework** monorepo. The working directory (`TestProject/`) is an Unreal Engine 5.7 project used to test and validate the framework. Source code lives in the parent directories:

```
NEXUS/
├── Plugins/        # Core framework plugins (Runtime + Editor modules, the source of truth))
├── Samples/        # Sample content plugins per feature area
├── TestProject/    # UE5.7 test project (this repo root for Claude Code)
│   ├── Source/NEXUS/   # Minimal game module (just module boilerplate)
│   └── Content/        # Test maps and assets
├── .github/        # CI/CD workflows and reusable actions
└── Staging/        # Build output and test results (generated)
```

The `TestProject` is a thin host project. Actual feature work lives in `../Plugins/<PluginName>/Source/`.

## Build

UE installation can be found at `C:\UE\UE_5.7` (defined in `.github/branch.json`), or `D:\EGS\UE_5.7`.

**Build (Editor target):**
```powershell
<UE Installation>\Engine\Build\BatchFiles\Build.bat NEXUSEditor Win64 Development "<this repo root>\NEXUS.uproject" -progress
```

**Build (Game target):**
```powershell
<UE Installation>\Engine\Build\BatchFiles\Build.bat NEXUS Win64 Development "<this repo root>\NEXUS.uproject" -progress
```

## Tests

Tests live in `Editor` module `Tests/` subdirectories (e.g., `Plugins/Core/Source/NexusCoreEditor/Tests/`). They are compiled only when `WITH_TESTS` is defined and run via the Unreal Automation Framework.

```powershell
# Unit tests
& "<UE Installation>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "TestProject\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests;Quit" -nullrhi

# Functional tests (requires RHI - no -nullrhi)
& "<UE Installation>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "TestProject\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest Tests.Nexus;Quit"

# Performance tests
& "<UE Installation>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "TestProject\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.PerfTests;Quit" -nullrhi
```

Results are written to `Staging/TestResults/index.json`.

## Plugin Architecture

Each plugin follows this layout:
```
Plugins/<Name>/
├── Source/
│   ├── Nexus<Name>/          # Runtime module (Public/ + Private/)
│   └── Nexus<Name>Editor/    # Editor module (includes Tests/ subdirectory)
└── Nexus<Name>.uplugin
```

Tests live in `Nexus<Name>Editor/Tests/` and are gated with `#if WITH_TESTS`.

Active plugins loaded by the TestProject:

| Plugin | Purpose |
|--------|---------|
| **NexusCore** | Shared base utilities, test macros, and helpers used by all other plugins |
| **NexusActorPools** | Generalized actor pooling system |
| **NexusBlockout** | Level blockout tools |
| **NexusDynamicRefs** | Dynamic asset reference system |
| **NexusGuardian** | Monitors UObject counts against a baseline; fires warnings/snapshots when configurable thresholds are crossed |
| **NexusMultiplayer** | Networking / multiplayer support |
| **NexusPicker** | Point distribution and picking utilities |
| **NexusProcGen** | Procedural generation (depends on UE's PCG plugin) |
| **NexusTooling** | Editor tools and fixers |
| **NexusUI** | UMG/Slate UI components (depends on CommonUI) |

`NexusCore` is a dependency of all other plugins and provides shared utilities: math helpers, collections, debug draw, actor/level libraries, and the developer-only headers in `Public/Developer/` (test utilities, object snapshots, scope timers, `NDebugActor`).

## Writing Tests

Tests belong in the `Editor` module under a `Tests/` subdirectory. Wrap all test code in `#if WITH_TESTS / #endif`.
Tests use the NEXUS test macros from `NexusCore/Public/Macros/NTestMacros.h`. All tests are prefixed `[NEXUS]` in the automation system.
Use `FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World) { ... })` when a test needs a world context.

```cpp
#if WITH_TESTS

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

// Priority macros: N_TEST_CRITICAL, N_TEST_HIGH, N_TEST_MEDIUM, N_TEST_LOW
// Filter macros: N_TEST_SMOKE_*, N_TEST_STRESS_*, N_TEST_PERF_*
// Context tags: N_TEST_CONTEXT_EDITOR, N_TEST_CONTEXT_ANYWHERE, N_TEST_CONTEXT_CLIENT, N_TEST_CONTEXT_SERVER

N_TEST_CRITICAL(FMyTests, "NEXUS::UnitTests::MyPlugin::MyTest", N_TEST_CONTEXT_EDITOR)
{
   
	// test body

}

#endif //WITH_TESTS
```

The test name string prefix `NEXUS::UnitTests` is what the CI automation filter `NEXUS.UnitTests` matches against.

## Code Style & Naming Conventions

Style is enforced via `.editorconfig`. Key rules:
- Tabs, width 4; max line length 150
- Inline brace style — no brace-on-new-line for namespaces, types, or functions
- Pointer/reference alignment: left (`int* Ptr`, not `int *Ptr`)
- Doxygen-style doc comments (`/** */`)
- Every `.cpp` and `.h` must start with:
  ```cpp
  // Copyright dotBunny Inc. All Rights Reserved.
  // See the LICENSE file at the repository root for more information.
  ```

Standard UE prefixes: `A` (AActor), `U` (UObject), `F` (structs), `E` (enums), `T` (templates), `S` (SWidget), `b` (bool fields). NEXUS adds `N` after the type prefix: `ANDebugActor`, `FNActorPool`, `UNDynamicRef`.

Each module has a `Nexus<Name>Minimal.h` that declares the module's log category (e.g. `LogNexusActorPools`, `LogNexusCoreEditor`). Include this instead of pulling in the full module header.

## NexusCore Macro Library

`NexusCore/Public/Macros/` contains framework-wide macros used across all plugins. The most important:

**Module & settings boilerplate** (`NModuleMacros.h`, `NSettingsMacros.h`):
- `N_IMPLEMENT_MODULE(Type, Name)` — adds a `static Type& Get()` accessor. Declare in the class body; keep `IMPLEMENT_MODULE()` in the `.cpp`.
- `N_IMPLEMENT_MODULE_POST_ENGINE_INIT(Type, Method)` — defers a startup method until after engine init.
- `N_IMPLEMENT_SETTINGS(Type)` — adds `static const Type* Get()` and `static Type* GetMutable()` to a `UDeveloperSettings` subclass.

**Subsystem helpers** (`NSubsystemMacros.h`):
- `N_WORLD_SUBSYSTEM(Type)` — adds `static Type* Get(const UWorld*)` and the `GetStatId()` implementation.
- `N_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate)` — same, but restricts creation to Game/PIE worlds.
- `N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate)` — for tickable world subsystems limited to gameplay.

**Bitflag helpers** (`NFlagsMacros.h`): `N_FLAGS_HAS`, `N_FLAGS_HAS_ANY_OF`, `N_FLAGS_HAS_NONE_OF`, `N_FLAGS_ADD`, `N_FLAGS_REMOVE`.

**Data structure helpers** (`NArrayMacros.h`): `N_IMPLEMENT_FLAT_2D_ARRAY` / `N_IMPLEMENT_FLAT_3D_ARRAY` — inject flat-array storage with typed `Get/SetData`, `GetIndex`, `GetInverseIndex`, and `GetSurroundingIndices` into a struct.

**Actor & world helpers**: `N_WORLD_ICON_HEADER/CLEANUP/IMPLEMENTATION` (`NActorMacros.h`) for editor billboard sprites on actors; `N_GET_WORLD_FROM_CONTEXT` (`NWorldMacros.h`) returns the world safely (uses `GEngine->GetWorldFromContextObject` in non-Shipping).

**Interface dispatch** (`NInterfaceMacros.h`): `N_INTERFACE_INVOKE_METHOD(InterfaceName, MethodName, EventName)` — dispatches to the native method or Blueprint event depending on whether the object is a C++ or Blueprint implementor.

## Cross-Cutting Patterns

**Subsystem access**: Each plugin exposes its runtime via `UNXxxSubsystem : public UTickableWorldSubsystem` using `N_WORLD_SUBSYSTEM_GAME_ONLY`. Call `UNXxxSubsystem::Get(World)` — never `GetSubsystem<>` directly.

**Developer Overlays**: Several plugins ship a `U*DeveloperOverlay` (extending `UNDeveloperOverlay` from NexusUI) as a runtime debug HUD widget. These are Blueprint-assignable `CommonUserWidget` subclasses with bindable status fields.

**Settings**: Plugin-level settings derive from `UDeveloperSettings` and use `N_IMPLEMENT_SETTINGS`, giving a cached `::Get()` singleton. Project-level overrides live in `Config/`.

**Samples vs Tests**: Functional test content lives in `Samples/<Name>/Content/` as separate plugins (`NexusXxxSamples`). Unit tests live in `Plugins/<Name>/Source/Nexus<Name>Editor/Tests/`. The two test categories use different exec commands (`Tests.Nexus` vs `NEXUS.UnitTests`).

## CI/CD

Workflows are in `.github/workflows/`. The self-hosted runner expects the UE installation path from `.github/branch.json`.

- **push-unit-tests.yml** — triggers on pushes to `main` that touch `Plugins/**/Source/**` or `TestProject/Source/**`
- **manual-buildcookrun.yml** / **dispatch-buildcookrun.yml** — BuildCookRun (Development/DebugGame/Shipping)
- **manual-tests.yml** / **dispatch-functional-tests.yml** — functional test runs
- **schedule-performance-tests.yml** — scheduled perf test runs

Reusable actions are in `.github/actions/` (build-project, ue-cmd-tests, workspace-clean, ue-parse-log, ue-parse-results, upload-artifacts).
