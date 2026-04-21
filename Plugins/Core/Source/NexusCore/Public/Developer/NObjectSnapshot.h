// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NObjectSnapshotEntry.h"
#include "NObjectSnapshot.generated.h"

/**
 * A captured, point-in-time record of the UObjects alive in the global object array.
 *
 * Created by FNObjectSnapshotUtils::Snapshot(). Pairs of snapshots can be diffed with
 * FNObjectSnapshotUtils::Diff() to produce an FNObjectSnapshotDiff describing what appeared or
 * disappeared between captures — the core primitive used by the framework's leak-detection tooling.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshot
{
	GENERATED_BODY()

	FNObjectSnapshot() = default;

	/**
	 * Creates an empty snapshot with storage reserved for Count entries.
	 * @param Count The expected number of entries; used only as a reservation hint for CapturedObjects.
	 */
	explicit FNObjectSnapshot(const int32 Count)
		: Ticket(-1), CapturedObjectCount(0), UntrackedObjectCount(0)
	{
		CapturedObjects.Reserve(Count);
	}

	/** Monotonic identifier assigned when the snapshot was produced; -1 for an uninitialized snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 Ticket = -1;

	/** Number of UObjects whose details were recorded in CapturedObjects. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 CapturedObjectCount = 0;

	/** Number of UObjects that were visible to the scan but deliberately not tracked (e.g. filtered). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 UntrackedObjectCount = 0;

	/** The recorded per-UObject entries captured at snapshot time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<FNObjectSnapshotEntry> CapturedObjects;

	/** Restores the snapshot to an empty, uninitialized state, freeing the CapturedObjects array. */
	void Reset()
	{
		Ticket = -1;
		CapturedObjectCount = 0;
		UntrackedObjectCount = 0;
		CapturedObjects.Empty();
	}

	/**
	 * Returns a one-line textual summary of the snapshot's capture counts.
	 * @return A string in the form "Captured N Objects (U Untracked)".
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Captured %i Objects (%i Untracked)"), CapturedObjectCount, UntrackedObjectCount);
	}

	/**
	 * Returns a multi-line textual summary that includes every captured entry's description.
	 * @return The detailed string; one header line followed by one line per captured entry.
	 */
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

	/** Writes a detailed summary of the snapshot to LogNexusCore, one entry per line. */
	void DumpToLog()
	{
		UE_LOG(LogNexusCore, Log, TEXT("[FNObjectSnapshot]"));
		UE_LOG(LogNexusCore, Log, TEXT("Captured %i Objects (%i Untracked)"), CapturedObjectCount, UntrackedObjectCount);
		for (const FNObjectSnapshotEntry& Entry : CapturedObjects)
		{
			UE_LOG(LogNexusCore, Log, TEXT("  %s"), *Entry.ToString());
		}
	}
};
