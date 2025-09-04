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
		return FString::Printf(TEXT("Total %i (%i Changes) - Added %i / Maintained %i / Removed %i"), ObjectCount, ChangeCount, AddedCount, MaintainedCount, RemovedCount);
	}

	FString ToDetailedString()
	{
		FStringBuilderBase StringBuilder;

		StringBuilder.Appendf(TEXT("Captured %i Objects (%i Changes)\n"), ObjectCount, ChangeCount);
		
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

	FString ToMarkdown()
	{
		FStringBuilderBase StringBuilder;

		StringBuilder.Appendf(TEXT("# Captured %i Objects (%i Changes)\n"), ObjectCount, ChangeCount);

		StringBuilder.Appendf(TEXT("## Added (%d):\n"), AddedCount);
		StringBuilder.Append(TEXT("|Ref Count|Full Name\n|:--|:--|\n"));
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			StringBuilder.Appendf(TEXT("%s\n"), *Entry.ToMarkdownTableRow());
		}
		
		StringBuilder.Appendf(TEXT("## Maintained (%d):\n"), MaintainedCount);
		StringBuilder.Append(TEXT("|Row|Ref Count|Full Name\n|:--|:--|\n"));
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			StringBuilder.Appendf(TEXT("%s\n"), *Entry.ToMarkdownTableRow());
		}
		
		StringBuilder.Appendf(TEXT("## Removed (%d):\n"), RemovedCount);
		StringBuilder.Append(TEXT("|Ref Count|Full Name\n|:--|:--|\n"));
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			StringBuilder.Appendf(TEXT("|%i%s\n"), *Entry.ToMarkdownTableRow());
		}

		return StringBuilder.ToString();
	}

	void DumpToLog()
	{
		N_LOG(Log, TEXT("%s"), *FString::Printf(TEXT("[FNObjectSnapshotDiff::DumpToLog] Captured %i Objects (%i Changes)"), ObjectCount, ChangeCount));
		
		N_LOG(Log, TEXT("Added (%i):"), AddedCount);
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}

		N_LOG(Log, TEXT("Maintained (%i):"), MaintainedCount);
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}

		N_LOG(Log, TEXT("Removed (%i):"), RemovedCount);
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			N_LOG(Log, TEXT("  %s"), *Entry.ToString());
		}
	}
};
