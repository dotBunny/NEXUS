// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotEntry.h"
#include "NObjectSnapshot.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshot
{
	GENERATED_BODY()

	FNObjectSnapshot() = default;
	
	explicit FNObjectSnapshot(const int32 Count)
		: Ticket(-1)
		, CapturedObjectCount(0)
	{
		CapturedObjects.Reserve(Count);
	}

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 Ticket = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 CapturedObjectCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> CapturedObjects;
	
	FString ToString() const
	{
		return FString::Printf(TEXT("Captured %i Objects"), CapturedObjectCount);
	}
};
