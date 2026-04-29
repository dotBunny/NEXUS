// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Lightweight scoped timer used by ProcGen analytics structs to measure individual stages.
 *
 * Calling Start() captures the current platform time; Stop() records the end time and stores the
 * elapsed duration in milliseconds. Intended for logging/debug only — no thread-safety guarantees.
 */
struct FNProcGenTaskTimer
{
	/** Platform time (seconds) at which Start was last called. */
	double StartTime;

	/** Platform time (seconds) at which Stop was last called. */
	double EndTime;

	/** Elapsed time between the most recent Start/Stop pair, in milliseconds. */
	double Duration;

	/** Capture the current platform time as the start of a new measurement window. */
	void Start()
	{
		StartTime = FPlatformTime::Seconds();
	}

	/** Capture the current platform time as the end of the measurement window and update Duration. */
	void Stop()
	{
		EndTime = FPlatformTime::Seconds();
		Duration = (EndTime -StartTime) * 1000.f;
	}
};