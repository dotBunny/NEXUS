// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NObjectSnapshotUtils.h"

class NEXUSCORE_API FNMethodScopeObjectDiff
{
public:
	explicit FNMethodScopeObjectDiff(const FString& InName) : Name(InName), StartSnapshot(FNObjectSnapshotUtils::Snapshot())
	{
	}
    
	~FNMethodScopeObjectDiff()
	{
		const FNObjectSnapshot EndSnapshot = FNObjectSnapshotUtils::Snapshot();
		const FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(StartSnapshot, EndSnapshot, false);
		
		N_LOG("[FNMethodScopeObjectDiff] %s : %s", *Name, *Diff.ToString());
	}
private:
	FString Name;
	FNObjectSnapshot StartSnapshot;
	
};
