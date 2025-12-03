// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NObjectSnapshotEntry.h"
#include "NObjectSnapshot.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshot
{
	GENERATED_BODY()

	FNObjectSnapshot() = default;
	
	explicit FNObjectSnapshot(const int32 Count)
		: Ticket(-1), CapturedObjectCount(0), UntrackedObjectCount(0)
	{
		CapturedObjects.Reserve(Count);
	}

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 Ticket = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 CapturedObjectCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 UntrackedObjectCount = 0;
	

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> CapturedObjects;

	void Reset()
	{
		Ticket = -1;
		CapturedObjectCount = 0;
		UntrackedObjectCount = 0;
		CapturedObjects.Empty();
	}
	
	FString ToString() const
	{
		return FString::Printf(TEXT("Captured %i Objects (%i Untracked)"), CapturedObjectCount, UntrackedObjectCount);
	}

	FString ToDetailedString() const
	{
		FStringBuilderBase StringBuilder;
		StringBuilder.Appendf(TEXT("Captured %i Objects (%i Untracked)\n"), CapturedObjectCount, UntrackedObjectCount);
		for (const FNObjectSnapshotEntry& Entry : CapturedObjects)
		{
			StringBuilder.Appendf(TEXT("  %s\n"), *Entry.ToString());
		}
		return StringBuilder.ToString();
	}

	void DumpToLog()
	{
		N_LOG("%s", *FString::Printf(TEXT("[FNObjectSnapshot::DumpToLog] Captured %i Objects (%i Untracked)"), CapturedObjectCount, UntrackedObjectCount));
		for (const FNObjectSnapshotEntry& Entry : CapturedObjects)
		{
			N_LOG("  %s", *Entry.ToString());
		}
	}
};
