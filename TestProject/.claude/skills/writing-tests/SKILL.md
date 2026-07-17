---
name: writing-tests
description: Creating new unit tests, test cases, or performance tests. Invoke this skill whenever writing tests for any NEXUS plugin.
---

# Writing Tests

Follow the `coding-style` skill for all formatting, naming, and doc comment conventions — test files are subject to the same rules as production code.

## File Placement & Naming

- Tests live in dedicated **test modules** under the plugin's `Source/`, split by what they depend on (which is also the shippability boundary):
  - **Runtime tests** → `Plugins/<Name>/Source/Nexus<Name>Tests/Private/` — a `DeveloperTool` module. Use this when the test touches only runtime/shippable code (the `Nexus<Name>` runtime module + `NexusCore`).
  - **Editor tests** → `Plugins/<Name>/Source/Nexus<Name>EditorTests/Private/` — an `Editor` module. Use this **only** when the test references an editor-module symbol — anything from `Nexus<Name>Editor`, `NexusCoreEditor`, `UnrealEd`, editor subsystems, data validators, etc.
  - Decision rule: if the file compiles against only runtime headers it belongs in `Nexus<Name>Tests`; a **single** editor-module include pushes it to `Nexus<Name>EditorTests`.
- A plugin has whichever of the two modules it needs (often just `Nexus<Name>Tests`). If the required test module doesn't exist yet, create it — `Nexus<Name>Tests.Build.cs` (or `…EditorTests`) + `Public/` & `Private/` module `.h`/`.cpp` (`N_MODULE_BASE` + `IMPLEMENT_MODULE`) — then register it in the plugin's `.uplugin` `Modules` array with `"Type": "DeveloperTool"` (runtime) or `"Type": "Editor"` (editor). Model it on an existing one: `NexusActorPoolsTests` (runtime) or `NexusUIEditorTests` (editor). An editor test module must depend on the editor module(s) it includes; a runtime test module must not depend on any editor module.
- **Reflected test-helper types** — a `UCLASS`/`UINTERFACE` used only by tests (e.g. `NTestPooledActor.h`, `NInterfaceMacrosTestTypes.h`) — must be guarded with `#if WITH_EDITORONLY_DATA`, **not** `#if WITH_TESTS`: UHT rejects `UFUNCTION`s inside a `WITH_TESTS` scope. Plain (non-reflected) helper headers may use `#if WITH_TESTS`. Tag any exported helper with the test module's own API macro (`NEXUS<UPPERCASEPLUGIN>TESTS_API`), not the editor module's.
- File naming is based on the **class being tested**:
  - `<ClassName>Tests.cpp` — all tests for one class
  - `<ClassName>Tests_<Feature>.cpp` — tests for a specific feature area
  - `<ClassName>PerfTests.cpp` / `<ClassName>PerfTests_<Feature>.cpp` — performance tests
- Examples: `FNActorPool` → `NActorPoolTests.cpp`; `UNDynamicRefSubsystem` → `NDynamicRefSubsystemTests_Delegates.cpp`; `INActorPoolItem` → `INActorPoolItemTests.cpp`

## Every File Must Start With

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS
```

And end with:

```cpp
#endif //WITH_TESTS
```

## Required Includes

```cpp
// Always needed — also transitively pulls in every FNTestLatentCommand_* class
// and the N_TESTS_PERF_START_LATENT_TEST* / _FINISH_LATENT_TEST* macros, so
// perf tests do NOT need a separate latent-commands include.
#include "Macros/NTestMacros.h"

// Needed for WorldTest / WorldTestChecked (unit tests)
#include "Developer/NTestUtils.h"

// Needed for CHECK_EQUALS only when NTestUtils.h is NOT included
// (NTestUtils.h pulls it in transitively)
#include "Tests/TestHarnessAdapter.h"
```

Include plugin-specific headers and the module's `Nexus<Name>Minimal.h` as needed.

## Test Name Hierarchy

| Subject | Struct name | String name |
|---|---|---|
| `FNActorPool` – leak check, force destroy | `FNActorPoolTests_LeakCheck_ForceDestroy` | `"NEXUS::UnitTests::NActorPools::FNActorPool::LeakCheck::ForceDestroy"` |
| `UNDynamicRefSubsystem` – fast collection, add | `UNDynamicRefSubsystemTests_FastCollection_AddObject` | `"NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::FastCollection::AddObject"` |
| `FNMersenneTwister` – float range | `FNMersenneTwisterTests_Float_Range` | `"NEXUS::UnitTests::NCore::FNMersenneTwister::Float_Range"` |

Rules:
- **Unit tests**: `"NEXUS::UnitTests::<ShortPlugin>::<ClassName>::<Category>::<Case>"`
- **Perf tests**: `"NEXUS::PerfTests::<ShortPlugin>::<ClassName>::<Operation>"`
- `<ShortPlugin>` drops `Nexus` and replaces with `N`: `NexusDynamicRefs` → `NDynamicRefs`, `NexusActorPools` → `NActorPools`, `NexusCore` → `NCore`
- `<ClassName>` in the string always includes the UE type prefix (`F`, `U`, `I`) when the tested subject has one
- The CI filter `NEXUS.UnitTests` matches all unit tests; `NEXUS.PerfTests` matches all perf tests

## Macro Reference

### Priority macros (choose one per test)

| Macro | Use when |
|---|---|
| `N_TEST_CRITICAL` | Core invariants; must never fail |
| `N_TEST_HIGH` | Important but not blocking |
| `N_TEST_MEDIUM` | Standard coverage |
| `N_TEST_LOW` | Edge cases, nice-to-have |
| `N_TEST` | No priority qualifier needed (legacy / simple cases) |

### Filter macros (combine with priority, e.g. `N_TEST_PERF_HIGH`)

| Filter family | Macro suffix | Purpose |
|---|---|---|
| Smoke | `N_TEST_SMOKE_*` | Runs on editor startup |
| Stress | `N_TEST_STRESS_*` | High-load / repetition tests |
| Performance | `N_TEST_PERF_*` | Timing-bounded perf tests |
| Engine | `N_TEST_ENGINE_*` | Engine integration |
| Product | `N_TEST_PRODUCT_*` | Product/feature-level |
| Failure | `N_TEST_FAILURE_*` | Negative / expected-failure tests |

### Context tags (third argument)

| Tag | Use when |
|---|---|
| `N_TEST_CONTEXT_EDITOR` | Requires editor world or editor subsystems |
| `N_TEST_CONTEXT_ANYWHERE` | Pure logic, no world needed |
| `N_TEST_CONTEXT_CLIENT` | Client-only context |
| `N_TEST_CONTEXT_SERVER` | Server-only context |
| `N_TEST_CONTEXT_CLIENT_SERVER` | Both client and server |
| `N_TEST_CONTEXT_COMMANDLET` | Commandlet context |

## Assertion Macros

```cpp
// Equality — most common for value comparisons (plain string, not TEXT())
CHECK_EQUALS("message describing the assertion", actual, expected)

// Boolean true/false with a message (use TEXT() macro for the string)
CHECK_MESSAGE(TEXT("message"), expression)
CHECK_FALSE_MESSAGE(TEXT("message"), expression)

// Pointer / identity checks — use ADD_ERROR inside an if-block
if (SomePtr != ExpectedPtr)
{
    ADD_ERROR("message describing what went wrong");
}

// Nullptr safety check before proceeding
if (SomePtr == nullptr)
{
    ADD_ERROR("message");
    return; // or return from lambda
}
```

## Unit Test Format — Pure Logic (`N_TEST_CONTEXT_ANYWHERE`)

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "MyPluginHeader.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNMyClassTests_Feature_Case,
    "NEXUS::UnitTests::NMyPlugin::FNMyClass::Feature::Case",
    N_TEST_CONTEXT_ANYWHERE)
{
    const int32 Result = FNMyClass::SomePureFunction(42);
    CHECK_MESSAGE(TEXT("Expected result for input 42."), Result == 84);
}

#endif //WITH_TESTS
```

If the test needs a helper struct, declare it in a namespace that mirrors the test hierarchy, above the first test:

```cpp
namespace NEXUS::UnitTests::NMyPlugin::FNMyClassHarness
{
    struct FTestFixture
    {
        int32 Value = 0;
    };
}
```

## Unit Test Format — World-Based (`N_TEST_CONTEXT_EDITOR`)

Use `WorldTestChecked` for unit tests (validates pre/post world state conditions). Use `WorldTest` when explicit GC control is needed; pass `true` as the final argument to disable GC during the test.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "MyPluginHeader.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(UNMySubsystemTests_Category_Case,
    "NEXUS::UnitTests::NMyPlugin::UNMySubsystem::Category::Case",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that <brief one-line description of what this test checks>.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNMySubsystem* Subsystem = UNMySubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNMySubsystem from editor world.");
            return;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->DoThing(Actor);

        CHECK_EQUALS("Count should be 1 after adding one object.", Subsystem->GetCount(), 1)

        if (Subsystem->GetFirst() != Actor)
        {
            ADD_ERROR("GetFirst did not return the expected actor.");
        }
    });
}

#endif //WITH_TESTS
```

## Performance Test Format — Pure Logic (`N_TEST_CONTEXT_ANYWHERE`)

Performance tests run through the latent-automation framework so the engine can stabilize before timing starts. Wrap the timed work in a static method on a `class FN<ClassName>PerfTests` and dispatch from each `N_TEST_PERF` body via `FNTestLatentCommand`. Constants live in a `NEXUS::PerfTests::<ShortPlugin>::<ClassName>Harness` namespace above the class. Always end the timed region with `NTestTimer.ManualStop()` inside the `N_TEST_TIMER_SCOPE` block.

> **Wrapper class name**: always `FN<ClassName>PerfTests`, even when the subject is `U`-prefixed (e.g. `UNDynamicRefSubsystem` → `class FNDynamicRefSubsystemPerfTests`). The wrapper is a plain C++ class, never a `UCLASS`.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "MyPluginHeader.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NMyPlugin::FNMyClassHarness
{
    constexpr float MaxDuration = 0.2f;
}

class FNMyClassPerfTests
{
public:
    static void OperationName()
    {
        FNMyClass Subject(SomeSetupArg);

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNMyClassPerfTests_OperationName,
                NEXUS::PerfTests::NMyPlugin::FNMyClassHarness::MaxDuration)
            for (int32 i = 0; i < 10000; ++i)
            {
                Subject.DoWork();
            }
            NTestTimer.ManualStop();
        }
    }
};

N_TEST_PERF(FNMyClassPerfTests_OperationName,
    "NEXUS::PerfTests::NMyPlugin::FNMyClass::OperationName",
    N_TEST_CONTEXT_ANYWHERE)
{
    N_TESTS_PERF_START_LATENT_TEST
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNMyClassPerfTests::OperationName));
    N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
```

## Performance Test Format — World-Based (`N_TEST_CONTEXT_EDITOR`)

Same shape as the pure-logic variant, but each static method takes a `UWorld*` and the test body uses the `_WORLD` macro pair plus `FNTestLatentCommand_WorldTest`. The macros handle world creation, GC suppression, and cleanup — never call `FNTestUtils::WorldTest` or `Pre/PostPerformanceTest` from inside the test body. Pass `true` as the second arg to `FNTestLatentCommand_WorldTest` to disable GC during the test (the standard choice for perf tests). The `UWorld*` parameter must be non-`const` to match the latent-command signature.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "MyPluginHeader.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness
{
    constexpr int32 ObjectCount = 1000;
    constexpr float AddMaxDuration = 0.5f;
    constexpr float RemoveMaxDuration = 1.0f;
}

class FNMySubsystemPerfTests
{
public:
    // Measures the cost of adding ObjectCount unique actors to a single bucket.
    static void AddObject(UWorld* World)
    {
        UNMySubsystem* Subsystem = UNMySubsystem::Get(World);
        if (!Subsystem) return;

        // Pre-spawn outside the timed region.
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::ObjectCount);
        for (int32 i = 0; i < NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::ObjectCount; ++i)
        {
            Actors.Add(World->SpawnActor<AActor>());
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNMySubsystemPerfTests_AddObject,
                NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::AddMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::ObjectCount; ++i)
            {
                Subsystem->AddObject(Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }
};

N_TEST_PERF(UNMySubsystemPerfTests_AddObject,
    "NEXUS::PerfTests::NMyPlugin::UNMySubsystem::AddObject",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNMySubsystemPerfTests::AddObject, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

#endif //WITH_TESTS
```

## Performance Test Format — Asserting Inside the Timed Body

`ADD_ERROR`, `CHECK_MESSAGE`, and `CHECK_EQUALS` need a `Test` symbol in scope. The plain `FNTestLatentCommand` / `FNTestLatentCommand_WorldTest` static methods don't have one, so use the `_WithBase` variants when an assertion has to live inside the static method:

| Need | Latent command | Static method signature |
|---|---|---|
| Pure logic, no assertions | `FNTestLatentCommand` | `static void M()` |
| Pure logic, with assertions | `FNTestLatentCommand_WithBase` | `static void M(FAutomationTestBase* Test)` |
| World-based, no assertions | `FNTestLatentCommand_WorldTest` | `static void M(UWorld* World)` |
| World-based, with assertions | `FNTestLatentCommand_WorldTestWithBase` | `static void M(UWorld* World, FAutomationTestBase* Test)` |

The parameter **must be named `Test`** — the assertion macros expand to expressions referencing that exact name. Default to the non-`_WithBase` variants and only switch when an assertion is actually required; keep correctness checks in unit tests where possible.

```cpp
class FNMySubsystemPerfTests
{
public:
    static void GetActorPool(UWorld* World, FAutomationTestBase* Test)
    {
        UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
        if (!Subsystem) return;

        FNActorPool* LastPool = nullptr;
        // TEST
        {
            N_TEST_TIMER_SCOPE(UNMySubsystemPerfTests_GetActorPool,
                NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::GetMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NMyPlugin::UNMySubsystemHarness::QueryCount; ++i)
            {
                LastPool = Subsystem->GetActorPool(AActor::StaticClass());
            }
            NTestTimer.ManualStop();
        }

        if (LastPool == nullptr)
        {
            ADD_ERROR("GetActorPool returned nullptr for a pool that was just created.");
            return;
        }
        CHECK_MESSAGE(TEXT("GetActorPool should return the templated pool."),
            LastPool->GetTemplate() == AActor::StaticClass())
    }
};

N_TEST_PERF(UNMySubsystemPerfTests_GetActorPool,
    "NEXUS::PerfTests::NMyPlugin::UNMySubsystem::GetActorPool",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTestWithBase(
        &FNMySubsystemPerfTests::GetActorPool, true, this));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}
```

## Common Patterns

- **Debug Actor**: Use `ANDebugActor` instead of `AActor`.
- **Subsystem null guard in unit tests**: `ADD_ERROR` + `return` inside the lambda.
- **Subsystem null guard in perf tests**: bare `if (!Subsystem) return;` — no `ADD_ERROR` needed.
- **Brief intent comment**: Start the test/lambda body with `// Verifies that …` when the test name alone doesn't make the intent obvious.
- **Reading existing tests before writing**: Always read the existing test files for the target plugin first (in `Nexus<Name>Tests/` and/or `Nexus<Name>EditorTests/`) to match established patterns and avoid conflicting fixture values.
