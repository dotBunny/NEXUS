// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"

/**
 * A scoped wall-clock timer that logs its elapsed duration when it falls out of scope.
 *
 * Place one of these on the stack at the top of a region you wish to time; its destructor will
 * emit a single LogNexusCore line in milliseconds. Call ManualStop() earlier to freeze the
 * timer before destruction if the region ends in a non-scope-aligned control flow.
 */
class FNMethodScopeTimer
{
public:
	/**
	 * Records the start time and caches a label used in the final log line.
	 * @param InName Human-readable label included in the destructor log line.
	 */
	explicit FNMethodScopeTimer(const FString& InName) : Name(InName), StartTime(FPlatformTime::Seconds()), EndTime(0)
	{
	}

	/** Freezes the captured end time immediately; subsequent calls (including destruction) are no-ops. */
	void ManualStop()
	{
		if (!bStopped)
		{
			EndTime = FPlatformTime::Seconds();
			bStopped = true;
		}
	}

	/** Stops the timer if still running and logs the elapsed duration in milliseconds to LogNexusCore. */
	~FNMethodScopeTimer()
	{
		ManualStop();
		const double DurationMs = (EndTime - StartTime) * 1000.0;
		UE_LOG(LogNexusCore, Log, TEXT("FNMethodScopeTimer(%s) took %f ms."), *Name, DurationMs);
	}

private:
	bool bStopped = false;
	FString Name;
	double StartTime;
	double EndTime;
};
