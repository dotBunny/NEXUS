// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NObjectSnapshotUtils.h"

/**
 * A scoped helper that captures a UObject snapshot at construction and logs the diff at destruction.
 *
 * Drop one of these onto the stack at the top of a function or code region to get a one-line
 * summary of the objects created and removed during that scope. It is purely a diagnostic aid
 * and should not be left enabled in shipped code.
 * @note Taking two full UObject snapshots is expensive; use only in targeted debug sessions.
 */
class NEXUSCORE_API FNMethodScopeObjectDiff
{
public:
	/**
	 * Captures a baseline UObject snapshot labelled with InName.
	 * @param InName Human-readable label included in the destructor log line.
	 */
	explicit FNMethodScopeObjectDiff(const FString& InName) : Name(InName), StartSnapshot(FNObjectSnapshotUtils::Snapshot())
	{
	}

	/** Captures a second snapshot, diffs it against the baseline and writes the summary to LogNexusCore. */
	~FNMethodScopeObjectDiff()
	{
		const FNObjectSnapshot EndSnapshot = FNObjectSnapshotUtils::Snapshot();
		const FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(StartSnapshot, EndSnapshot, false);

		UE_LOG(LogNexusCore, Log, TEXT("FNMethodScopeObjectDiff(%s) %s", *Name, *Diff.ToString());
	}
private:
	FString Name;
	FNObjectSnapshot StartSnapshot;

};
