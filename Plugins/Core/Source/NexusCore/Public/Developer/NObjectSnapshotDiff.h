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
	int32 UntrackedObjectCountA = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 UntrackedObjectCountB = 0;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 ChangeCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 ObjectCount = 0;

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
		return FString::Printf(TEXT("Total %i (%i Changes | %i Previously Untracked | %i Currently Untracked) - Added %i / Maintained %i / Removed %i"),
			ObjectCount, ChangeCount, UntrackedObjectCountA, UntrackedObjectCountB, AddedCount, MaintainedCount, RemovedCount);
	}

	FString ToDetailedString() const
	{
		FStringBuilderBase StringBuilder;

		StringBuilder.Appendf(TEXT("Captured %i Objects (%i Changes)\n"), ObjectCount, ChangeCount);
		StringBuilder.Appendf(TEXT("Previously %i Untracked Objects | Currently %i Untracked Objects\n"), UntrackedObjectCountA, UntrackedObjectCountB);
		
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

	void DumpToLog() const
	{
		UE_LOG(LogNexusCore, Log, TEXT("[FNObjectSnapshotDiff]"));
		UE_LOG(LogNexusCore, Log, TEXT("Captured %i Objects (%i Changes)"), ObjectCount, ChangeCount);
		UE_LOG(LogNexusCore, Log, TEXT("Previously %i Untracked Objects | Currently %i Untracked Objects"), UntrackedObjectCountA, UntrackedObjectCountB);
		UE_LOG(LogNexusCore, Log, TEXT("Added (%i):"), AddedCount);
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			UE_LOG(LogNexusCore, Log, TEXT("  %s"), *Entry.ToString());
		}

		UE_LOG(LogNexusCore, Log, TEXT("Maintained (%i):"), MaintainedCount);
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			UE_LOG(LogNexusCore, Log, TEXT("  %s"), *Entry.ToString());
		}

		UE_LOG(LogNexusCore, Log, TEXT("Removed (%i):"), RemovedCount);
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			UE_LOG(LogNexusCore, Log, TEXT("  %s"), *Entry.ToString());
		}
	}
};
