---
name: Generating Unit Test
description:  Creating new unit tests, test cases, or performance tests.
---

# Generating Unit Test

When writing tests, utilize these guidelines:

1. Tests belong in the `Editor` module under a `Tests/` subdirectory (e.g., `Plugins/Core/Source/NexusCoreEditor/Tests/`).
2. Wrap all test code in `#if WITH_TESTS / #endif`.
3. Tests use the NEXUS test macros from `NexusCore/Public/Macros/NTestMacros.h`.
4. All tests are prefixed `[NEXUS]` in the automation system.
4. Use `FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World) { ... })` when a test needs a world context.
5. Use the `CHECK_MESSAGE` macro instead of `CHECK_TRUE` when providing a message.
6. Use the `CHECK_FALSE_MESSAGE` macro instead of `CHECK_FALSE` when providing a message. 
7. The test name string prefix `NEXUS::UnitTests` is what the CI automation filter `NEXUS.UnitTests` matches against.
8. The `MyPlugin` name is a shortened version of the Plugins full name, for example `NexusDynamicRefs` becomes `NDynamicRefs`, replacing `Nexus` with just the letter `N`.
9. Try to follow the patterns established in other plugins established tests.

##  Unit Tests Format

```cpp
#if WITH_TESTS

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// Priority macros: N_TEST_CRITICAL, N_TEST_HIGH, N_TEST_MEDIUM, N_TEST_LOW
// Filter macros: N_TEST_SMOKE_*, N_TEST_STRESS_*, N_TEST_PERF_*
// Context tags: N_TEST_CONTEXT_EDITOR, N_TEST_CONTEXT_ANYWHERE, N_TEST_CONTEXT_CLIENT, N_TEST_CONTEXT_SERVER

N_TEST_CRITICAL(FMyPluginTests, "NEXUS::UnitTests::MyPlugin::MyTest", N_TEST_CONTEXT_EDITOR)
{   
	// test body
}

#endif //WITH_TESTS
```

## Performance Tests Format

```cpp
#if WITH_TESTS

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// Priority macros: N_TEST_PERF_CRITICAL, N_TEST_PERF_HIGH, N_TEST_PERF_MEDIUM, N_TEST_PERF_LOW
// Filter macros: N_TEST_SMOKE_*, N_TEST_STRESS_*, N_TEST_PERF_*
// Context tags: N_TEST_CONTEXT_EDITOR, N_TEST_CONTEXT_ANYWHERE, N_TEST_CONTEXT_CLIENT, N_TEST_CONTEXT_SERVER

N_TEST_PERF_CRITICAL(FMyPluginPerfTests, "NEXUS::PerfTests::MyPlugin::MyTest", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::PrePerformanceTest();
	// test body
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
```