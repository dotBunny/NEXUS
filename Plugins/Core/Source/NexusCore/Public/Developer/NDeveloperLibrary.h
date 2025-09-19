// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NDeveloperUtils.h"
#include "NObjectSnapshotUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDeveloperLibrary.generated.h"

/**
 * A small collection of functionality to wrap the usage of developer tools.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/developer-library/">UNDeveloperLibrary</a>
 */
UCLASS()
class UNDeveloperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the current number of known UObjects by looking at the global UObject array and subtracting the number of available spots.
	 * @return The number of objects.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Count", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-count"))
	static int32 GetCurrentObjectCount() { return FNDeveloperUtils::GetCurrentObjectCount(); };

	/**
	 * Captures a minimal data structure about all currently known UObjects.
	 * @return The FNObjectSnapshot recording structure.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#create-uobject-snapshot"))
	static FNObjectSnapshot CreateObjectSnapshot() { return FNObjectSnapshotUtils::Snapshot(); }

	/**
	 * Compares two already created snapshots to formulate a difference report of what changed.
	 * @param OldSnapshot Original FNObjectSnapshot created as the baseline.
	 * @param NewSnapshot The FNObjectSnapshot to compare the baseline against.
	 * @param bRemoveKnownLeaks Should known objects that leak be removed from tracking.
	 * @return A FNObjectSnapshotDiff structure outlining the differences.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot Diff", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#create-uobject-snapshot-dff"))
	static FNObjectSnapshotDiff CreateSnapshotDiff(const FNObjectSnapshot& OldSnapshot, const FNObjectSnapshot& NewSnapshot, const bool bRemoveKnownLeaks = false)
	{
		return FNObjectSnapshotUtils::Diff(OldSnapshot, NewSnapshot, bRemoveKnownLeaks);
	}

	/**
	 * Get an output-friendly string from an FNObjectSnapshotEntry.
	 * @param Entry The entry to generate a string from.
	 * @return The output-friendly string.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Entry Summary", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-snapshot-entry-summary"))
	static FString GetObjectSnapshotEntrySummary(const FNObjectSnapshotEntry& Entry) { return Entry.ToString(); }

	/**
	 * Get an output-friendly string that summarizes the contents of an FNObjectSnapshot.
	 * @param Snapshot The snapshot to generate a string from.
	 * @return The output-friendly string.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Summary", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-snapshot-summary"))
	static FString GetObjectSnapshotSummary(const FNObjectSnapshot& Snapshot) { return Snapshot.ToString(); }

	/**
	 * Get an output-friendly string that provides a more in-depth output detailing the contents of an FNObjectSnapshot.
	 * @param Snapshot The snapshot to generate a detailed string from.
	 * @return The output-friendly string.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Detailed Summary", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-snapshot-detailed-summary"))
	static FString GetObjectSnapshotDetailedSummary(const FNObjectSnapshot& Snapshot) { return Snapshot.ToDetailedString(); }
	
	/**
	 * Get an output-friendly string that summarizes the contents of an FNObjectSnapshotDiff.
	 * @param Diff The FNObjectSnapshotDiff to generate a string from.
	 * @return The output-friendly string.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Diff Summary", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-snapshot-diff-summary"))
	static FString GetObjectSnapshotDiffSummary(const FNObjectSnapshotDiff& Diff) { return Diff.ToString(); }

	/**
	 * Get an output-friendly string that provides a more in-depth output detailing the contents of an FNObjectSnapshotDiff.
	 * @param Diff The FNObjectSnapshotDiff to generate a detailed string from.
	 * @return The output-friendly string.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Diff Detailed Summary", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#get-uobject-snapshot-diff-detailed-summary"))
	static FString GetObjectSnapshotDiffDetailedSummary(const FNObjectSnapshotDiff& Diff) { return Diff.ToDetailedString(); }

	/**
	 * Output an FNObjectSnapshotDiff's detailed summary to LogNexus.
	 * @param Diff The FNObjectSnapshotDiff to generate output from.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Output Snapshot To Log", Category = "NEXUS|Developer",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-library/#output-snapshot-to-log"))
	static void DumpSnapshotDiffToLog(const FNObjectSnapshotDiff& Diff) { Diff.DumpToLog(); }
};