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
	static int GetCurrentObjectCount() { return FNDeveloperUtils::GetCurrentObjectCount(); };

	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot", Category = "NEXUS|Developer")
	static FNObjectSnapshot CreateObjectSnapshot() { return FNObjectSnapshotUtils::Snapshot(); }

	UFUNCTION(BlueprintCallable, DisplayName = "Create UObject Snapshot Diff", Category = "NEXUS|Developer")
	static FNObjectSnapshotDiff CreateSnapshotDiff(const FNObjectSnapshot& OldSnapshot, const FNObjectSnapshot& NewSnapshot, const bool bRemoveKnownLeaks = false)
	{
		return FNObjectSnapshotUtils::Diff(OldSnapshot, NewSnapshot, bRemoveKnownLeaks);
	}
};