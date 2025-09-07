// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NDeveloperUtils.h"
#include "NObjectSnapshotUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDeveloperLibrary.generated.h"

UCLASS()
class UNDeveloperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Count", Category = "NEXUS|Developer")
	static int32 GetCurrentObjectCount() { return FNDeveloperUtils::GetCurrentObjectCount(); };

	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot", Category = "NEXUS|Developer")
	static FNObjectSnapshot CreateObjectSnapshot() { return FNObjectSnapshotUtils::Snapshot(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot Diff", Category = "NEXUS|Developer")
	static FNObjectSnapshotDiff CreateSnapshotDiff(const FNObjectSnapshot& OldSnapshot, const FNObjectSnapshot& NewSnapshot, const bool bRemoveKnownLeaks = false)
	{
		return FNObjectSnapshotUtils::Diff(OldSnapshot, NewSnapshot, bRemoveKnownLeaks);
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Entry Summary", Category = "NEXUS|Developer")
	static FString GetObjectSnapshotEntrySummary(const FNObjectSnapshotEntry& Entry) { return Entry.ToString(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Summary", Category = "NEXUS|Developer")
	static FString GetObjectSnapshotSummary(const FNObjectSnapshot& Snapshot) { return Snapshot.ToString(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Detailed Summary", Category = "NEXUS|Developer")
	static FString GetObjectSnapshotDetailedSummary(const FNObjectSnapshot& Snapshot) { return Snapshot.ToDetailedString(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Diff Summary", Category = "NEXUS|Developer")
	static FString GetObjectSnapshotDiffSummary(const FNObjectSnapshotDiff& Diff) { return Diff.ToString(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Get UObject Snapshot Diff Detailed Summary", Category = "NEXUS|Developer")
	static FString GetObjectSnapshotDiffDetailedSummary(const FNObjectSnapshotDiff& Diff) { return Diff.ToDetailedString(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Output Snapshot To Log", Category = "NEXUS|Developer")
	static void DumpSnapshotDiffToLog(const FNObjectSnapshotDiff& Diff) { Diff.DumpToLog(); }
};