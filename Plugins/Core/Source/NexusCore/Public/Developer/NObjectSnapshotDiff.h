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
		FStringBuilderBase StringBuilder;

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

	void DumpToLog()
	{
		N_LOG(Log, TEXT(" %s"), *FString::Printf(TEXT("[FNObjectSnapshotDiff::DumpToLog] Captured %i Objects"), CapturedObjectCount));

		N_LOG(Log, TEXT("Added (%d):"), AddedCount);
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}

		N_LOG(Log, TEXT("Maintained (%d):"), MaintainedCount);
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}

		N_LOG(Log, TEXT("Removed (%d):"), RemovedCount);
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}
	}
};
