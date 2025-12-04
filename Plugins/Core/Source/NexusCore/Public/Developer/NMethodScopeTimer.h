// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"

class FNMethodScopeTimer
{
public:
	explicit FNMethodScopeTimer(const FString& InName) : Name(InName), StartTime(FPlatformTime::Seconds()), EndTime(0)
	{
	}

	void ManualStop()
	{
		if (!bStopped)
		{
			EndTime = FPlatformTime::Seconds();
			bStopped = true;
		}
	}
	 
	~FNMethodScopeTimer()
	{
		ManualStop();
		const double DurationMs = (EndTime - StartTime) * 1000.0;
		UE_LOG(LogNexusCore, Log, TEXT("FNMethodScopeTimer(%s) took %f ms", *Name, DurationMs);
	}

private:
	bool bStopped = false;
	FString Name;
	double StartTime;
	double EndTime;
};
