// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNProcGenTaskTimer
{
	double StartTime;
	double EndTime;
	double Duration;
	
	void Start()
	{
		StartTime = FPlatformTime::Seconds();
	}
	void Stop()
	{
		EndTime = FPlatformTime::Seconds();
		Duration = (EndTime -StartTime) * 1000.f;
	}
};