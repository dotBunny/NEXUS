// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "NCoreMinimal.h"
#include "Misc/LowLevelTestAdapter.h"

#if !WITH_LOW_LEVEL_TESTS
#include "Misc/AutomationTest.h"
#endif

/**
 * A scoped test-aware timer that integrates with the Low-Level Test framework.
 *
 * Like FNMethodScopeTimer but with extra test-harness hooks: the measured duration is reported via
 * INFO() and, if MaxDurationMs is exceeded, the enclosing test is failed with ADD_ERROR(). When
 * enabled, a FPlatformMisc named event is emitted so the region is visible in profilers such as
 * Unreal Insights.
 *
 * @note When ran on a build machine (env.IsBuildMachine=1) a warning is NOT thrown, instead we print an INFO statement.
 *		 Telemetry will capture the time and bubble it up.
 */
class FNTestScopeTimer
{
public:
	/**
	 * Starts the timer, optionally emitting a named-event marker.
	 * @param InName Human-readable label included in logs, the INFO line and the failure message.
	 * @param MaxDurationMs Fail threshold in milliseconds. The test errors if the scope exceeds this.
	 * @param bUseNamedEvent When true, wraps the scope in a FPlatformMisc named event for profilers.
	 * @param InContext Optional grouping label forwarded to AddTelemetryData so Gauntlet can bucket results.
	 */
	explicit FNTestScopeTimer(const FString& InName, const float MaxDurationMs = MAX_FLT, const bool bUseNamedEvent = true, const FString& InContext = TEXT("NEXUS"))
		: bNamedEvent(bUseNamedEvent), Name(InName), Context(InContext), MaxDuration(MaxDurationMs)
	{
		if (bUseNamedEvent)
		{
			UE_LOG(LogNexusCore, Log, TEXT("[%s] BEGIN"), *Name);
			FPlatformMisc::BeginNamedEvent(FColor::Blue, *Name);
		}

		// Manually set start-time after logging
		StartTime =  FPlatformTime::Seconds();
	}

	/**
	 * Freezes the captured end time and closes the named event if one was opened.
	 * Subsequent calls (including destruction) are no-ops.
	 */
	void ManualStop()
	{
		if (!bStopped)
		{
			EndTime = FPlatformTime::Seconds();
			if (bNamedEvent)
			{
				FPlatformMisc::EndNamedEvent();
				UE_LOG(LogNexusCore, Log, TEXT("[%s] END"), *Name);
			}
			bStopped = true;
		}
	}

	/** Stops the timer if still running, reports the duration to the test log, and fails the test if the threshold was exceeded. */
	~FNTestScopeTimer()
	{
		ManualStop();

		const double DurationMs = (EndTime - StartTime) * 1000.0;
		INFO(FString::Printf(TEXT("[%s] %f ms"), *Name, DurationMs));

#if !WITH_LOW_LEVEL_TESTS
		if (GIsAutomationTesting)
		{
			if (FAutomationTestBase* CurrentTest = FAutomationTestFramework::Get().GetCurrentTest())
			{
				CurrentTest->AddTelemetryData(Name, DurationMs, Context);
			}
		}
#endif

		if (DurationMs > MaxDuration)
		{
			if(FPlatformMisc::GetEnvironmentVariable(TEXT("IsBuildMachine")) != TEXT("1"))
			{
				ADD_ERROR(FString::Printf(TEXT("[%s] %f ms EXCEEDS %f defined MaxDuration."), *Name, DurationMs, MaxDuration));
			}
			else
			{
				INFO(FString::Printf(TEXT("[%s] %f ms EXCEEDS %f defined MaxDuration."), *Name, DurationMs, MaxDuration));
			}
		}
	}

private:
	bool bNamedEvent = true;
	bool bStopped = false;
	FString Name;
	FString Context;
	double MaxDuration;
	double StartTime;
	double EndTime = 0;
};

#endif // WITH_TESTS