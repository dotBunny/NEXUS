// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NObjectSnapshotEntry.h"
#include "NObjectSnapshotDiff.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshotDiff
{

	GENERATED_BODY()

	FNObjectSnapshotDiff() = default;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 CapturedObjectCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Added;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 AddedCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Maintained;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 MaintainedCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Removed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 RemovedCount = 0;

	FString ToString() const
	{
		return FString::Printf(TEXT("Added: %d, Maintained: %d, Removed: %d"), AddedCount, MaintainedCount, RemovedCount);
	}

	FString ToDetailedString()
	{

		TStringBuilder<512> StringBuilder;

		StringBuilder.Appendf(TEXT("Captured Object Count: %d\n"), CapturedObjectCount);

		StringBuilder.Appendf(TEXT("Added (%d):\n"), AddedCount);
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			StringBuilder.Appendf(TEXT("  %s\n"), *Entry.ToString());
		}

		StringBuilder.Appendf(TEXT("Maintained (%d):\n"), MaintainedCount);
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			StringBuilder.Appendf(TEXT("  %s\n"), *Entry.ToString());
		}

		StringBuilder.Appendf(TEXT("Removed (%d):\n"), RemovedCount);
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			StringBuilder.Appendf(TEXT("  %s\n"), *Entry.ToString());
		}

		return StringBuilder.ToString();
		

	}
};
