// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NDelayedEditorTask.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorUserSettings.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "NLeakTestDelayedEditorTask.generated.h"

/**
 * Delayed editor task that snapshots live UObjects, waits LeakCheckTime seconds, then diffs
 * against a second snapshot. A growth in the Added count is written to Saved/Logs as a leak
 * report; a clean run is logged at Log severity.
 */
UCLASS()
class UNLeakTestDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	/** Schedule a new leak-test run using the LeakCheckTime configured in user settings. */
	static void Create()
	{
		UAsyncEditorDelay* DelayedMechanism = CreateDelayMechanism();
		UNLeakTestDelayedEditorTask* LeakTestObject = NewObject<UNLeakTestDelayedEditorTask>(DelayedMechanism);
		LeakTestObject->Lock(DelayedMechanism);

		DelayedMechanism->Complete.AddDynamic(LeakTestObject, &UNLeakTestDelayedEditorTask::Execute);
		LeakTestObject->BeforeSnapshot = FNObjectSnapshotUtils::Snapshot();

		DelayedMechanism->Start(UNToolingEditorUserSettings::Get()->LeakCheckTime, 100);
	}

private:
	/** Snapshot of the UObject table captured at task creation, used as the diff baseline. */
	FNObjectSnapshot BeforeSnapshot;

	/** Delay-complete callback — re-snapshot, diff, and write a report if Added > 0. */
	UFUNCTION()
	void Execute()
	{
		const FNObjectSnapshot AfterSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(BeforeSnapshot, AfterSnapshot, true);
		
		if (Diff.AddedCount > 0)
		{
			const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_LeakCheck_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
			FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
			UE_LOG(LogNexusToolingEditor, Warning, TEXT("UObject leak detected; comparison written to %s."), *DumpFilePath);
		}
		else
		{
			UE_LOG(LogNexusToolingEditor, Log, TEXT("No measurable UObject leak was detected. %s"), *Diff.ToString());
		}
		Release();
	}
};