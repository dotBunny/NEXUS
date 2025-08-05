// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NCoreEditorMinimal.h"
#include "NDelayedEditorTask.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "NLeakTestDelayedEditorTask.generated.h"

UCLASS()
class NEXUSCOREEDITOR_API UNLeakTestDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	static void Create()
	{
		UAsyncEditorDelay* DelayedMechanism = CreateDelayMechanism();
		UNLeakTestDelayedEditorTask* LeakTestObject = NewObject<UNLeakTestDelayedEditorTask>(DelayedMechanism);
		LeakTestObject->Lock(DelayedMechanism);
		
		DelayedMechanism->Complete.AddDynamic(LeakTestObject, &UNLeakTestDelayedEditorTask::Execute);
		LeakTestObject->BeforeSnapshot = FNObjectSnapshotUtils::Snapshot();
		DelayedMechanism->Start(5.f, 100);
	}
	
private:
	FNObjectSnapshot BeforeSnapshot;
	
	UFUNCTION()
	void Execute()
	{
		const FNObjectSnapshot AfterSnapshot = FNObjectSnapshotUtils::Snapshot();

		// Process
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(BeforeSnapshot, AfterSnapshot, true);
		if (Diff.AddedCount > 0)
		{
			NE_LOG(Error, TEXT("[FNEditorCommands::OnToolsLeakCheck] %s"), *Diff.ToDetailedString());
		}
		else
		{
			NE_LOG(Log, TEXT("[FNEditorCommands::OnToolsLeakCheck] %s"), *Diff.ToString());
		}

		Release();
	}
};