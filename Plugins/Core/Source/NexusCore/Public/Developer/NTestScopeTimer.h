// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/LowLevelTestAdapter.h"

class FNTestScopeTimer
{
public:
	explicit FNTestScopeTimer(const FString& InName, const float MaxDurationMs = MAX_FLT, const bool bUseNamedEvent = true)
		: Name(InName), MaxDuration(MaxDurationMs), StartTime(FPlatformTime::Seconds())
	{
		bNamedEvent = bUseNamedEvent;
		if (bNamedEvent)
		{
			
			FPlatformMisc::BeginNamedEvent(FColor::Blue, *Name);
		}
	}
	
	~FNTestScopeTimer()
	{
		if (bNamedEvent)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] END"), *Name);
			FPlatformMisc::EndNamedEvent();
		}
		const double EndTime = FPlatformTime::Seconds();
		const double DurationMs = (EndTime - StartTime) * 1000.0;
		
		if (DurationMs >= MaxDuration)
		{
			ADD_ERROR(FString::Printf(TEXT("[%s] %f ms EXCEEDS %f defined MaxDuration."), *Name, DurationMs, MaxDuration));
		}
		else
		{
			INFO(FString::Printf(TEXT("[%s] %f ms"), *Name, DurationMs));
		}
	}

private:
	bool bNamedEvent = true;
	FString Name;
	double MaxDuration;
	double StartTime;
};
