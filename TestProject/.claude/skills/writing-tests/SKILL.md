---
name: writing-tests
description: Creating new unit tests, test cases, or performance tests. Invoke this skill whenever writing tests for any NEXUS plugin.
---

# Writing Tests

Follow the `coding-style` skill for all formatting, naming, and doc comment conventions — test files are subject to the same rules as production code.

When writing tests, follow these guidelines exactly.

## File Placement & Naming

- Tests belong in the `Editor` module under a `Tests/` subdirectory:
  `Plugins/<Name>/Source/Nexus<Name>Editor/Tests/`
- File naming is based on the **class being tested**, not the plugin short name:
  - `<ClassName>Tests.cpp` — all tests for one class in one file
  - `<ClassName>Tests_<Feature>.cpp` — tests for a specific feature area of a class
  - `<ClassName>PerfTests.cpp` — performance tests for a class
  - `<ClassName>PerfTests_<Feature>.cpp` — perf tests for a specific feature area
- `<ClassName>` carries the same UE type prefix as the tested subject:
  - `FNActorPool` → `NActorPoolTests.cpp`, `NActorPoolPerfTests.cpp`
  - `UNDynamicRefSubsystem` → `NDynamicRefSubsystemTests.cpp`, `NDynamicRefSubsystemTests_Delegates.cpp`
  - `FNMersenneTwister` → `NMersenneTwisterTests.cpp`, `NMersenneTwisterPerfTests.cpp`
  - `INActorPoolItem` → `INActorPoolItemTests.cpp`
  - `NFlagsMacros` (macro group, no prefix) → `NFlagsMacrosTests.cpp`

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

Include only what is needed:

```cpp
// Always needed
#include "Macros/NTestMacros.h"

// Needed for WorldTest / WorldTestChecked / PrePerformanceTest / PostPerformanceTest
#include "Developer/NTestUtils.h"

// Needed for CHECK_EQUALS only when NTestUtils.h is NOT included
// (NTestUtils.h pulls it in transitively, so skip this when already including NTestUtils.h)
#include "Tests/TestHarnessAdapter.h"
```

Include plugin-specific headers as needed (e.g. `"NActorPool.h"`, `"NDynamicRefSubsystem.h"`).

Include the module's `Nexus<Name>Minimal.h` rather than the full module header.

## Test Name Hierarchy

Test struct names and string names mirror the class under test:

| Subject | Struct name | String name |
|---|---|---|
| `FNActorPool` – leak check, force destroy | `FNActorPoolTests_LeakCheck_ForceDestroy` | `"NEXUS::UnitTests::NActorPools::FNActorPool::LeakCheck::ForceDestroy"` |
| `UNDynamicRefSubsystem` – fast collection, add | `UNDynamicRefSubsystemTests_FastCollection_AddObject` | `"NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::FastCollection::AddObject"` |
| `FNMersenneTwister` – float range | `FNMersenneTwisterTests_Float_Range` | `"NEXUS::UnitTests::NCore::FNMersenneTwister::Float_Range"` |
| `NFlagsMacros` – has bits set | `NFlagsMacrosTests_Has_BitsSet` | `"NEXUS::UnitTests::NCore::NFlagsMacros::Has_BitsSet"` |

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

For tests that need no world. Only `NTestMacros.h` is required. Use `CHECK_EQUALS` only if you also include `TestHarnessAdapter.h`.

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
namespace NEXUS::UnitTests::NMyPlugin::FNMyClass
{
    struct FTestFixture
    {
        int32 Value = 0;
    };
}
```

## Unit Test Format — World-Based (`N_TEST_CONTEXT_EDITOR`)

Use `WorldTestChecked` when correctness of the world state matters (validates pre/post conditions). `NTestUtils.h` is required.

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

Use `WorldTest` (without "Checked") when you need explicit GC control or accept less strict world validation. Pass `true` as the final argument to force world teardown after the lambda.

## Performance Test Format — Pure Logic (`N_TEST_CONTEXT_ANYWHERE`)

For perf tests with no world dependency. Place setup and the timed scope directly in the test body.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "MyPluginHeader.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF_HIGH(FNMyClassPerfTests_OperationName,
    "NEXUS::PerfTests::NMyPlugin::FNMyClass::OperationName",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    FNMyClass Subject(SomeSetupArg);
    {
        N_TEST_TIMER_SCOPE(FNMyClassPerfTests_OperationName, 10.f)
        for (int32 i = 0; i < 10000; ++i)
        {
            Subject.DoWork();
        }
    }

    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
```

## Performance Test Format — World-Based (`N_TEST_CONTEXT_EDITOR`)

Declare constants in a `namespace NEXUS::PerfTests::<ShortPlugin>::<ClassName>` block above the tests. Use `WorldTest(..., true)` (the `true` forces world teardown). Mark the timed region with a `// TEST` comment. Call `NTestTimer.ManualStop()` inside the timer scope when cleanup follows inside the same scope; omit it when the scope naturally ends after the measured work.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "MyPluginHeader.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NMyPlugin::UNMySubsystem
{
    constexpr int32 ObjectCount = 1000;
    constexpr float AddMaxDuration = 0.5f;
    constexpr float RemoveMaxDuration = 1.0f;
}

N_TEST_PERF(UNMySubsystemPerfTests_AddObject,
    "NEXUS::PerfTests::NMyPlugin::UNMySubsystem::AddObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of adding ObjectCount unique actors to a single bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNMySubsystem* Subsystem = UNMySubsystem::Get(World);
        if (!Subsystem) return;

        // Pre-spawn outside the timed region.
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::PerfTests::NMyPlugin::UNMySubsystem::ObjectCount);
        for (int32 i = 0; i < NEXUS::PerfTests::NMyPlugin::UNMySubsystem::ObjectCount; ++i)
        {
            Actors.Add(World->SpawnActor<AActor>());
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNMySubsystemPerfTests_AddObject,
                NEXUS::PerfTests::NMyPlugin::UNMySubsystem::AddMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NMyPlugin::UNMySubsystem::ObjectCount; ++i)
            {
                Subsystem->AddObject(Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
```

## Common Patterns

**Multiple tests in one file**: Group related tests in a single file. Each test still gets its own `N_TEST_*` block.

**Subsystem null guard in unit tests**: Always check the subsystem pointer immediately inside the lambda and `ADD_ERROR` + `return` if null.

**Subsystem null guard in perf tests**: Use a bare early-out — `if (!Subsystem) return;` — without `ADD_ERROR`. Perf tests don't need to record a failure message for this case.

**`WorldTestChecked` vs `WorldTest`**: Use `WorldTestChecked` for unit tests (validates world state pre/post). Use `WorldTest(..., true)` for perf tests and for unit tests that need explicit GC control.

**`ManualStop()`**: Call `NTestTimer.ManualStop()` inside the timer scope when there is cleanup code (e.g., `Pool.Clear()`) that should not be included in the timing. Omit it when the measured work ends right before the closing brace of the scope.

**Brief intent comment**: Start the test body (or lambda body for world tests) with a `// Verifies that …` comment when the test name alone doesn't make the intent obvious. Especially valuable for edge-case and delegate tests.

**Reading existing tests before writing**: Always read the existing `Tests/` files for the target plugin before writing new ones to match established patterns and avoid conflicting enum/name values used as test fixtures.
