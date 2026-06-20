// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NGeneralMacros.h"

#include "Developer/NTestScopeTimer.h"

// We're going to include our
#include "Developer/TestLatentCommands/NTestLatentCommand.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_CleanupWorld.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_CreateWorld.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_FrameWait.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_PostPerformanceTest.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_PrePerformanceTest.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_WithBase.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_WorldTest.h"
#include "Developer/TestLatentCommands/NTestLatentCommand_WorldTestWithBase.h"

/**
 * Macros that wrap Unreal's TEST_CASE_NAMED macro to declare NEXUS test cases.
 *
 * Every NEXUS test is tagged with [NEXUS] and (optionally) a priority ([CriticalPriority],
 * [HighPriority], [MediumPriority], [LowPriority]) plus one or more filter tags (Smoke, Stress,
 * Perf, Engine, Product, Negative). Context tags can be combined with the Flags argument at the
 * call site to further scope a test to a particular environment.
 */

/** Run-anywhere context; the test is valid in any environment. */
#define N_TEST_CONTEXT_ANYWHERE [AnywhereContext]
/** Client-only context. */
#define N_TEST_CONTEXT_CLIENT [ClientContext]
/** Combined client+server context. */
#define N_TEST_CONTEXT_CLIENT_SERVER [ClientContext][ServerContext]
/** Server-only context. */
#define N_TEST_CONTEXT_SERVER [ServerContext]
/** Editor-only context. */
#define N_TEST_CONTEXT_EDITOR [EditorContext]
/** Commandlet-only context. */
#define N_TEST_CONTEXT_COMMANDLET [CommandletContext]

/**
 * Declares a NEXUS-tagged test case with no default priority.
 * @param ClassName Identifier used for the test's generated class.
 * @param PrettyName Human-readable name shown in the test browser.
 * @param Flags Additional tag string (context, priority, filter) concatenated onto [NEXUS].
 */
#define N_TEST(ClassName, PrettyName, Flags)\
	TEST_CASE_NAMED(ClassName, PrettyName, N_STRINGIFY([NEXUS]Flags))

#define N_TEST_CRITICAL(ClassName, PrettyName, Flags)\
	TEST_CASE_NAMED(ClassName, PrettyName, N_STRINGIFY([NEXUS][CriticalPriority]Flags))
#define N_TEST_HIGH(ClassName, PrettyName, Flags)\
	TEST_CASE_NAMED(ClassName, PrettyName, N_STRINGIFY([NEXUS][HighPriority]Flags))
#define N_TEST_MEDIUM(ClassName, PrettyName, Flags)\
	TEST_CASE_NAMED(ClassName, PrettyName, N_STRINGIFY([NEXUS][MediumPriority]Flags))
#define N_TEST_LOW(ClassName, PrettyName, Flags)\
	TEST_CASE_NAMED(ClassName, PrettyName, N_STRINGIFY([NEXUS][LowPriority]Flags))

// Smoke tests run when starting the editor
#define N_TEST_SMOKE(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [SmokeFilter]Flags)
#define N_TEST_SMOKE_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [SmokeFilter]Flags)
#define N_TEST_SMOKE_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [SmokeFilter]Flags)
#define N_TEST_SMOKE_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [SmokeFilter]Flags)
#define N_TEST_SMOKE_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [SmokeFilter]Flags)

#define N_TEST_STRESS(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [StressFilter]Flags)
#define N_TEST_STRESS_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [StressFilter]Flags)
#define N_TEST_STRESS_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [StressFilter]Flags)
#define N_TEST_STRESS_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [StressFilter]Flags)
#define N_TEST_STRESS_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [StressFilter]Flags)

#define N_TEST_PERF(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [PerfFilter]Flags)
#define N_TEST_PERF_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [PerfFilter]Flags)
#define N_TEST_PERF_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [PerfFilter]Flags)
#define N_TEST_PERF_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [PerfFilter]Flags)
#define N_TEST_PERF_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [PerfFilter]Flags)

#define N_TEST_ENGINE(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [EngineFilter]Flags)
#define N_TEST_ENGINE_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [EngineFilter]Flags)
#define N_TEST_ENGINE_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [EngineFilter]Flags)
#define N_TEST_ENGINE_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [EngineFilter]Flags)
#define N_TEST_ENGINE_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [EngineFilter]Flags)

#define N_TEST_PRODUCT(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [ProductFilter]Flags)
#define N_TEST_PRODUCT_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [ProductFilter]Flags)
#define N_TEST_PRODUCT_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [ProductFilter]Flags)
#define N_TEST_PRODUCT_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [ProductFilter]Flags)
#define N_TEST_PRODUCT_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [ProductFilter]Flags)

#define N_TEST_FAILURE(ClassName, PrettyName, Flags)\
	N_TEST(ClassName, PrettyName, [NegativeFilter]Flags)
#define N_TEST_FAILURE_CRITICAL(ClassName, PrettyName, Flags)\
	N_TEST_CRITICAL(ClassName, PrettyName, [NegativeFilter]Flags)
#define N_TEST_FAILURE_HIGH(ClassName, PrettyName, Flags)\
	N_TEST_HIGH(ClassName, PrettyName, [NegativeFilter]Flags)
#define N_TEST_FAILURE_MEDIUM(ClassName, PrettyName, Flags)\
	N_TEST_MEDIUM(ClassName, PrettyName, [NegativeFilter]Flags)
#define N_TEST_FAILURE_LOW(ClassName, PrettyName, Flags)\
	N_TEST_LOW(ClassName, PrettyName, [NegativeFilter]Flags)


#define N_TEST_TIMER_SCOPE_1(Name)\
	TRACE_CPUPROFILER_EVENT_SCOPE(Name)\
	FNTestScopeTimer NTestTimer(TEXT(#Name), 0);


#define N_TEST_TIMER_SCOPE_2(Name, MaxDuration)\
	TRACE_CPUPROFILER_EVENT_SCOPE(Name)\
	FNTestScopeTimer NTestTimer(TEXT(#Name), MaxDuration);

#define N_TEST_TIMER_SCOPE_3(Name, MaxDuration, Context)\
	TRACE_CPUPROFILER_EVENT_SCOPE(Name)\
	FNTestScopeTimer NTestTimer(TEXT(#Name), MaxDuration, true, Context);

/** Internal selector that dispatches N_TEST_TIMER_SCOPE to the 2- or 3-arg variant based on argument count. */
#define N_TEST_TIMER_SCOPE_SELECT(_1, _2, _3, NAME, ...) NAME
/**
 * Opens a timed, test-aware scope.
 * @param Name Identifier stringified into the scope label and the telemetry data point name.
 * @param MaxDuration Fail threshold in milliseconds; the test errors if the scope exceeds this.
 * @param Context (optional) Telemetry grouping label forwarded to AddTelemetryData for Gauntlet.
 */
#define N_TEST_TIMER_SCOPE(...)\
	N_TEST_TIMER_SCOPE_SELECT(__VA_ARGS__, N_TEST_TIMER_SCOPE_3, N_TEST_TIMER_SCOPE_2, N_TEST_TIMER_SCOPE_1)(__VA_ARGS__)

namespace NEXUS::Testing::LatentCommands
{
	constexpr int32 PrePerformanceFrameWait = 45;
	constexpr int32 CreateWorldFrameWait = 10;
}

#define N_TESTS_PERF_START_LATENT_TEST \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PrePerformanceTest) \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(NEXUS::Testing::LatentCommands::PrePerformanceFrameWait)) \

#define N_TESTS_PERF_START_LATENT_TEST_WORLD \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PrePerformanceTest) \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(NEXUS::Testing::LatentCommands::PrePerformanceFrameWait)) \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_CreateWorld(this)) \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(NEXUS::Testing::LatentCommands::CreateWorldFrameWait))

#define N_TESTS_PERF_FINISH_LATENT_TEST \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PostPerformanceTest)

#define N_TESTS_PERF_FINISH_LATENT_TEST_WORLD \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_CleanupWorld) \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PostPerformanceTest)


