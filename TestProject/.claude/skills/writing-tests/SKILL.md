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
- File naming: `N<ShortPlugin>Tests_<Feature>.cpp` for unit tests, `N<ShortPlugin>PerfTests_<Feature>.cpp` for perf tests.
  - Example: `NActorPoolsTests_LeakCheck.cpp`, `NDynamicRefsPerfTests_Subsystem.cpp`
- `<ShortPlugin>` drops the `Nexus` prefix and replaces it with just `N`:
  - `NexusDynamicRefs` → `NDynamicRefs`, `NexusActorPools` → `NActorPools`, `NexusCore` → `NCore`

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

All test files need at minimum:

```cpp
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
```

Include `"Tests/TestHarnessAdapter.h"` when using `CHECK_EQUALS` — it is required for that macro to compile:

```cpp
#include "Tests/TestHarnessAdapter.h"
```

Include plugin-specific headers as needed (e.g. `"NActorPool.h"`, `"NDynamicRefSubsystem.h"`).

Include the module's `Nexus<Name>Minimal.h` rather than the full module header.

## Test Name Hierarchy

Unit test names follow: `"NEXUS::UnitTests::<ShortPlugin>::<Subsystem>::<What>"`
Perf test names follow: `"NEXUS::PerfTests::<ShortPlugin>::<What>"`

The CI filter `NEXUS.UnitTests` matches all unit tests; `NEXUS.PerfTests` matches all perf tests.

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

// Nullptr safety check
if (SomePtr == nullptr)
{
    ADD_ERROR("message");
    return; // or return from lambda
}
```

## Unit Test Format

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "MyPluginHeader.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNMyPluginTests_Feature_Case,
    "NEXUS::UnitTests::NMyPlugin::Feature::Case",
    N_TEST_CONTEXT_EDITOR)
{
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNMySubsystem* Subsystem = UNMySubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNMySubsystem from editor world.");
            return;
        }

        // Arrange & Act
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->DoThing(Actor);

        // Assert
        CHECK_EQUALS("Count should be 1 after adding one object.", Subsystem->GetCount(), 1)

        if (Subsystem->GetFirst() != Actor)
        {
            ADD_ERROR("GetFirst did not return the expected actor.");
        }
    });
}

#endif //WITH_TESTS
```

For tests that require no world (pure logic):

```cpp
N_TEST_HIGH(FNMyPluginTests_Feature_PureLogic,
    "NEXUS::UnitTests::NMyPlugin::Feature::PureLogic",
    N_TEST_CONTEXT_ANYWHERE)
{
    const int32 Result = SomePureFunction(42);
    CHECK_EQUALS("Expected result for input 42.", Result, 84)
}
```

## Performance Test Format

Declare constants in a `namespace NEXUS::<Plugin>PerfTests::<Feature>` block above the test. Use `N_TEST_PERF` (no priority qualifier) unless there is a strong reason to promote it. Use `FNTestUtils::WorldTest` (not `WorldTestChecked`) with `true` as the final argument to force world teardown. Wrap the measured section in `N_TEST_TIMER_SCOPE` and call `NTestTimer.ManualStop()` immediately after the measured work.

```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "MyPluginHeader.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::MyPluginTests::MyFeature
{
    constexpr int32 ObjectCount = 1000;
    constexpr float MaxDurationSeconds = 5.0f;
}

N_TEST_PERF(FNMyPluginPerfTests_MyFeature,
    "NEXUS::PerfTests::NMyPlugin::MyFeature",
    N_TEST_CONTEXT_EDITOR)
{
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        // Setup (not timed)
        FNMyStruct Setup = FNMyStruct(World);

        // Timed section
        {
            N_TEST_TIMER_SCOPE(FNMyPluginPerfTests_MyFeature, NEXUS::MyPluginTests::MyFeature::MaxDurationSeconds)
            for (int32 i = 0; i < NEXUS::MyPluginTests::MyFeature::ObjectCount; ++i)
            {
                Setup.DoWork();
            }
            NTestTimer.ManualStop();
        }

        Setup.Clear();
    }, true);
    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
```

## Common Patterns

**Multiple tests in one file**: Grouping related tests in a single file is fine. Each test still gets its own `N_TEST_*` block.

**Subsystem null-guard**: Always check the subsystem pointer immediately inside `WorldTestChecked` and `ADD_ERROR` + `return` if null — do not assert on it.

**`WorldTestChecked` vs `WorldTest`**: Use `WorldTestChecked` for unit tests (validates world state pre/post). Use `WorldTest(..., true)` for perf tests (the `true` argument forces world teardown after the lambda).

**Reading existing tests before writing**: Always read the existing `Tests/` files for the target plugin before writing new ones to match established patterns and avoid conflicting enum/name values used as test fixtures.
