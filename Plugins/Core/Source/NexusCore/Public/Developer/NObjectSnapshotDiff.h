// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NObjectSnapshotEntry.h"
#include "NObjectSnapshotDiff.generated.h"

/**
 * The result of diffing two FNObjectSnapshots.
 *
 * Produced by FNObjectSnapshotUtils::Diff(), this struct records which UObjects were added
 * between captures, which persisted across both captures, and which disappeared. It is the
 * primary data surface used by tests, the Guardian overlay, and developer tools to reason
 * about lifetime churn and suspected leaks.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshotDiff
{

	GENERATED_BODY()

	FNObjectSnapshotDiff() = default;

	/** Number of untracked objects recorded in the earlier (A) snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 UntrackedObjectCountA = 0;

	/** Number of untracked objects recorded in the later (B) snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 UntrackedObjectCountB = 0;

	/** Total of added plus removed entries — the net amount of churn between snapshots. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 ChangeCount = 0;

	/** Total number of entries examined across both snapshots. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 ObjectCount = 0;

	/** Entries that appear only in the later snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Added;

	/** Size of the Added array. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 AddedCount = 0;

	/** Entries that appear in both snapshots. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Maintained;

	/** Size of the Maintained array. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 MaintainedCount = 0;

	/** Entries that appear only in the earlier snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> Removed;

	/** Size of the Removed array. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 RemovedCount = 0;

	/**
	 * Returns a one-line summary suitable for logs and overlays.
	 * @return A string containing totals for changes, previously/currently untracked counts, and Added/Maintained/Removed.
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Total %i (%i Changes | %i Previously Untracked | %i Currently Untracked) - Added %i / Maintained %i / Removed %i"),
			ObjectCount, ChangeCount, UntrackedObjectCountA, UntrackedObjectCountB, AddedCount, MaintainedCount, RemovedCount);
	}

	/**
	 * Returns a multi-line summary that enumerates every Added, Maintained, and Removed entry.
	 * @return The detailed summary string, with one entry per line per category.
	 */
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
	
	/**
	 * Builds a structured report summarizing this diff (captured object count, change count, and the added, removed, and changed entries).
	 * @return An FNReport populated with the diff contents, ready for output to log or file.
	 */
	FNReport ToReport() const
	{
		FNReport Report;
		
		const int32 CapturedObjectsTicket = Report.CreateContentBlock();
		FNReportContentBlock* CapturedObjectBlock = Report.GetContentBlock(CapturedObjectsTicket);
		CapturedObjectBlock->SetHeading("FNObjectSnapshotDiff");
		
		CapturedObjectBlock->AddLine(FString::Printf(TEXT("Captured %i Objects"), ObjectCount));
		CapturedObjectBlock->AddLine(FString::Printf(TEXT("%i Changes"), ChangeCount));
		CapturedObjectBlock->AddLine(FString::Printf(TEXT("Previously %i Untracked Objects"), UntrackedObjectCountA));
		CapturedObjectBlock->AddLine(FString::Printf(TEXT("Currently %i Untracked Objects"), UntrackedObjectCountB));
		
		// ADDED
		const int32 AddedObjectsTableTicket = Report.CreateTableBlock(CapturedObjectsTicket);
		FNReportTableBlock* AddObjectsTableBlock = Report.GetTableBlock(AddedObjectsTableTicket);
		AddObjectsTableBlock->Initialize({"Full Name", "References", "Root Set", "Marked Garbage" });
		for (const FNObjectSnapshotEntry& Entry : Added)
		{
			AddObjectsTableBlock->AddRow({
				Entry.FullName,
				FString::FromInt(Entry.RefCount), 
				Entry.bIsRoot ? TEXT("R") : TEXT(""),
				Entry.bIsGarbage ? TEXT("G") : TEXT("")
			});
		}
		
		// MAINTAINED
		const int32 MaintainedObjectsTableTicket = Report.CreateTableBlock(CapturedObjectsTicket);
		FNReportTableBlock* MaintainedObjectsTableBlock = Report.GetTableBlock(MaintainedObjectsTableTicket);
		MaintainedObjectsTableBlock->Initialize({"Full Name", "References", "Root Set", "Marked Garbage" });
		for (const FNObjectSnapshotEntry& Entry : Maintained)
		{
			MaintainedObjectsTableBlock->AddRow({
				Entry.FullName,
				FString::FromInt(Entry.RefCount), 
				Entry.bIsRoot ? TEXT("R") : TEXT(""),
				Entry.bIsGarbage ? TEXT("G") : TEXT(""),
			});
		}
		
		// REMOVED
		const int32 RemovedObjectsTableTicket = Report.CreateTableBlock(CapturedObjectsTicket);
		FNReportTableBlock* RemovedObjectsTableBlock = Report.GetTableBlock(RemovedObjectsTableTicket);
		RemovedObjectsTableBlock->Initialize({"Full Name", "References", "Root Set", "Marked Garbage" });
		for (const FNObjectSnapshotEntry& Entry : Removed)
		{
			RemovedObjectsTableBlock->AddRow({
				Entry.FullName,
				FString::FromInt(Entry.RefCount), 
				Entry.bIsRoot ? TEXT("R") : TEXT(""),
				Entry.bIsGarbage ? TEXT("G") : TEXT("")
			});
		}
		
		return Report;
	}

	/** Writes a detailed summary of the diff to LogNexusCore, one category and one entry per line. */
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
