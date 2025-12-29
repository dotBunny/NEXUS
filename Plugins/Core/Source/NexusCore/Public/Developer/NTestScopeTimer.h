// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/LowLevelTestAdapter.h"

class FNTestScopeTimer
{
public:
	explicit FNTestScopeTimer(const FString& InName, const float MaxDurationMs = MAX_FLT, const bool bUseNamedEvent = true)
		: Name(InName), MaxDuration(MaxDurationMs), EndTime(0)
	{
		bNamedEvent = bUseNamedEvent;
		if (bNamedEvent)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] BEGIN"), *Name);
			FPlatformMisc::BeginNamedEvent(FColor::Blue, *Name);
		}
		
		// Manually set start-time after logging 
		StartTime =  FPlatformTime::Seconds();
	}

	void ManualStop()
	{
		if (!bStopped)
		{
			EndTime = FPlatformTime::Seconds();
			if (bNamedEvent)
			{
				FPlatformMisc::EndNamedEvent();
				UE_LOG(LogTemp, Log, TEXT("[%s] END"), *Name);
			}
			bStopped = true;
		}
	}
	
	~FNTestScopeTimer()
	{
		ManualStop();

		const double DurationMs = (EndTime - StartTime) * 1000.0;
		INFO(FString::Printf(TEXT("[%s] %f ms"), *Name, DurationMs));
		
		if (DurationMs >= MaxDuration)
		{
			ADD_ERROR(FString::Printf(TEXT("[%s] %f ms EXCEEDS %f defined MaxDuration."), *Name, DurationMs, MaxDuration));
		}
	}

private:
	bool bNamedEvent = true;
	bool bStopped = false;
	FString Name;
	double MaxDuration;
	double StartTime;
	double EndTime;
};
