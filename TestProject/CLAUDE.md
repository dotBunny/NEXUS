# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Layout

This is the **NEXUS Framework** monorepo. The working directory (`TestProject/`) is an Unreal Engine 5.7 project used to test and validate the framework.

Source code lives in the parent directories:
```
NEXUS/
├── Plugins/        # Core framework plugins (Runtime + Editor modules, the source of truth)
├── Samples/        # Sample content plugins per feature area
├── TestProject/    # UE5.7 test project (this repo root for Claude Code)
│   ├── Source/NEXUS/   # Minimal game module (just module boilerplate)
│   └── Content/        # Test maps and assets
├── .github/        # CI/CD workflows and reusable actions
└── Staging/        # Build output and test results (generated)
```

- The `TestProject` is a thin host project.
- Actual feature work lives in `../Plugins/<PluginName>/Source/`.
- The Unreal Engine source may be found in `C:\UE\UE_5.7`, `D:\UE\UE_5.7`, `E:\UE\UE_5.7`, or `D:\EGS\UE_5.7` depending on the computer. Before using any command that contains `___UEROOT___`, check each candidate path in order and use the first one that exists on disk.
- `___PROJECTROOT___` is the absolute path to the `TestProject/` directory — the current working directory. Resolve it from the working directory rather than hardcoding a path.

## Building

Build the `NEXUSEditor` target before running tests or opening the editor:

```powershell
& "___UEROOT___\Engine\Build\BatchFiles\Build.bat" NEXUSEditor Win64 Development "___PROJECTROOT___\NEXUS.uproject" -progress
```

| Configuration | Use when |
|---|---|
| `Development` | Day-to-day iteration |
| `DebugGame` | Stepping through game code in a debugger |
| `Shipping` | Release / packaging builds |

| Target | Description |
|---|---|
| `NEXUSEditor` | Editor target — local iteration and running tests |
| `NEXUS` | Game target — cook / package / shipping |

**Common build failures:**
- Missing module dependency — add to `PublicDependencyModuleNames` (headers in `.h`) or `PrivateDependencyModuleNames` (headers only in `.cpp`) in the plugin's `.Build.cs`.
- Missing API macro — exported class or function not tagged with `NEXUS<UPPERCASEPLUGINNAME>_API`.
- Stale generated files — delete `TestProject/Intermediate/` and rebuild.

Build log: `Saved/Logs/NEXUS.log`

## Running Tests

Build must be current before running tests.

```powershell
# All unit tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests;Quit" -nullrhi

# All functional tests (no -nullrhi — these require a real graphics context)
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest Tests.Nexus;Quit"

# All performance tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.PerfTests;Quit" -nullrhi
```

**Filtering:** Narrow to a single plugin by extending the filter prefix:

```powershell
-ExecCmds="Automation RunTest NEXUS.UnitTests.NActorPools;Quit"
```

**Single test** (use its fully-qualified name, which is the second argument to the `N_TEST_*` macro):

```powershell
-ExecCmds="Automation RunTest NEXUS::UnitTests::NActorPools::LeakCheck::DontForceDestroy;Quit"
```

Results are written to `Staging/TestResults/index.json`. Crash or unexpected failures: check `TestProject/Saved/Logs/NEXUS.log` for `Error:` lines.

## Plugin Architecture

Each plugin follows this layout:
```
Plugins/<Name>/
├── Source/
│   ├── Nexus<Name>/          # Runtime module (Public/ + Private/)
│   └── Nexus<Name>Editor/    # Editor module (includes Tests/ subdirectory)
└── Nexus<Name>.uplugin
```

The `.Build.cs` lives at `Plugins/<Name>/Source/Nexus<Name>/Nexus<Name>.Build.cs` (runtime) or `Plugins/<Name>/Source/Nexus<Name>Editor/Nexus<Name>Editor.Build.cs` (editor).

## Plugin Overview

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

## Coding Style

### File Headers

Every `.cpp` and `.h` must start with:
```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.
```

Every `.h` follows with `#pragma once`.

### Formatting

Style is enforced via `.editorconfig`:
- Tabs, width 4; max line length 150
- Inline brace style — no brace-on-new-line for namespaces, types, or functions
- Pointer/reference alignment: left (`int* Ptr`, not `int *Ptr`)

### Naming Conventions

Standard UE prefixes apply (`A`, `U`, `F`, `E`, `T`, `S`, `I`, `b`). NEXUS adds `N` after the type prefix:

| Example | Meaning |
|---|---|
| `ANDebugActor` | AActor subclass |
| `UNDynamicRef` | UObject subclass |
| `FNActorPool` | Struct |
| `ENActorComponentLifecycleEnd` | Enum |
| `INActorPoolItem` | Interface |

### Module API Macro

Every exported class and free function must be tagged with `NEXUS<UPPERCASEPLUGINNAME>_API`:
```cpp
class NEXUSACTORPOOLS_API UNActorPoolSubsystem : public UTickableWorldSubsystem { ... };
```

### Includes

- Include the module's `Nexus<Name>Minimal.h` rather than the full module header.
- Prefer forward declarations in headers over includes where only a pointer/reference is needed.

### Namespaces

Constants and helper data live in `namespace NEXUS::<PluginName>::<SubCategory>`. Do not put types or function definitions in these namespaces.

### UObject Members

Use `TObjectPtr<>` for all UObject references held as member variables.

### UCLASS / USTRUCT Specifiers

- `ClassGroup` is always `"NEXUS"`
- `DisplayName` format is `"NEXUS | <Human Readable Name>"`

### UFUNCTION Specifiers

- `Category` format is `"NEXUS|<PluginName>"` (no spaces around `|`)
- `DisplayName` is human-readable, title case, no "N" prefix
- Include `meta=(DocsURL="https://nexus-framework.com/docs/...")` on BlueprintCallable functions

### Editor-Only Members

Wrap editor-only fields and methods in `#if WITH_EDITOR ... #endif // WITH_EDITOR`.

### Interface Implementation Section Markers

Use `//~ClassName` and `//End ClassName` to delimit interface implementation blocks:
```cpp
//~UTickableWorldSubsystem
virtual void Tick(float DeltaTime) final override;
//End UTickableWorldSubsystem
```

### Doc Comments

Use Doxygen-style `/** */` for all public API:
```cpp
/**
 * Brief one-line description.
 * @param ParamName Description.
 * @return Description.
 * @note Thread-safety or behavioural caveats.
 * @remark "native code only" or manual-use warnings.
 * @see <a href="https://nexus-framework.com/docs/...">TypeName</a>
 */
```

## Writing Tests

Tests belong in the `Editor` module: `Plugins/<Name>/Source/Nexus<Name>Editor/Tests/`.

File naming is based on the class under test:
- `<ClassName>Tests.cpp` — all tests for one class
- `<ClassName>Tests_<Feature>.cpp` — tests for a specific feature area
- `<ClassName>PerfTests.cpp` — performance tests

`<ClassName>` carries the same UE type prefix as the tested subject (`FNActorPool` → `NActorPoolTests.cpp`).

### File Structure

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS
// ... includes and tests ...
#endif //WITH_TESTS
```

Required includes:
```cpp
#include "Macros/NTestMacros.h"               // always needed
#include "Developer/NTestUtils.h"              // needed for WorldTest / PrePerformanceTest
#include "Tests/TestHarnessAdapter.h"          // needed for CHECK_EQUALS only when NTestUtils.h is NOT included
```

### Test Name Hierarchy

- Unit tests: `"NEXUS::UnitTests::<ShortPlugin>::<ClassName>::<Category>::<Case>"`
- Perf tests: `"NEXUS::PerfTests::<ShortPlugin>::<ClassName>::<Operation>"`
- `<ShortPlugin>` drops `Nexus` and adds `N`: `NexusDynamicRefs` → `NDynamicRefs`, `NexusActorPools` → `NActorPools`, `NexusCore` → `NCore`

### Priority and Filter Macros

| Macro | Use when |
|---|---|
| `N_TEST_CRITICAL` | Core invariants |
| `N_TEST_HIGH` | Important but not blocking |
| `N_TEST_MEDIUM` | Standard coverage |
| `N_TEST_LOW` | Edge cases |
| `N_TEST_PERF_HIGH` etc. | Performance tests (combine `N_TEST_PERF_` with priority) |

Context tags: `N_TEST_CONTEXT_ANYWHERE` (pure logic), `N_TEST_CONTEXT_EDITOR` (requires editor world).

### Assertion Macros

```cpp
CHECK_EQUALS("message", actual, expected)           // value comparisons (plain string, not TEXT())
CHECK_MESSAGE(TEXT("message"), expression)          // boolean true
CHECK_FALSE_MESSAGE(TEXT("message"), expression)    // boolean false
ADD_ERROR("message")                                // pointer/identity checks inside if-blocks
```

### Unit Test — Pure Logic

```cpp
N_TEST_HIGH(FNMyClassTests_Feature_Case,
    "NEXUS::UnitTests::NMyPlugin::FNMyClass::Feature::Case",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_MESSAGE(TEXT("Expected result."), FNMyClass::SomePureFunction(42) == 84);
}
```

### Unit Test — World-Based

Use `FNTestUtils::WorldTestChecked` (validates pre/post world state). Always null-check the subsystem pointer and `ADD_ERROR` + `return` if null.

```cpp
N_TEST_CRITICAL(UNMySubsystemTests_Category_Case,
    "NEXUS::UnitTests::NMyPlugin::UNMySubsystem::Category::Case",
    N_TEST_CONTEXT_EDITOR)
{
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNMySubsystem* Subsystem = UNMySubsystem::Get(World);
        if (!Subsystem) { ADD_ERROR("Could not retrieve subsystem."); return; }
        // ... test body ...
    });
}
```

### Performance Tests

Wrap the timed region with `N_TEST_TIMER_SCOPE`. Call `FNTestUtils::PrePerformanceTest()` / `PostPerformanceTest()` around the test body. For world-based perf tests, use `WorldTest(..., true)` (the `true` disables GC). Call `NTestTimer.ManualStop()` inside the scope when cleanup code follows inside the same scope.

**Common patterns:**
- Use `ANDebugActor` instead of `AActor` when spawning test actors.
- Read existing `Tests/` files in the target plugin before writing new ones to match established patterns and avoid fixture value conflicts.
- Do not add section divider comments like `// ----- Feature ---`.
