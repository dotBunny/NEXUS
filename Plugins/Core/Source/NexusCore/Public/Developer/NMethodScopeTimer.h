// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"

class FNMethodScopeTimer
{
public:
	explicit FNMethodScopeTimer(const FString& InName) : Name(InName), StartTime(FPlatformTime::Seconds())
	{
	}
    
	~FNMethodScopeTimer()
	{
		const double EndTime = FPlatformTime::Seconds();
		const double DurationMs = (EndTime - StartTime) * 1000.0;
		
		N_LOG(Log, TEXT("[FNMethodScopeTimer] %s took: %f ms"), *Name, DurationMs);
	}

private:
	FString Name;
	double StartTime;
};
