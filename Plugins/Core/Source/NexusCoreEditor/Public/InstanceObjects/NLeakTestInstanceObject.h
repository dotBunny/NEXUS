// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NCoreEditorMinimal.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "NLeakTestInstanceObject.generated.h"

UCLASS()
class NEXUSCOREEDITOR_API UNLeakTestInstanceObject : public UObject
{
	GENERATED_BODY()

public:
	static void Create()
	{
		UAsyncEditorDelay* DelayedTest = NewObject<UAsyncEditorDelay>();
		UNLeakTestInstanceObject* LeakTestObject = NewObject<UNLeakTestInstanceObject>(DelayedTest);
		LeakTestObject->AddToRoot();
		
		DelayedTest->Complete.AddDynamic(LeakTestObject, &UNLeakTestInstanceObject::Execute);

		LeakTestObject->BeforeSnapshot = FNObjectSnapshotUtils::Snapshot();
		DelayedTest->Start(5.f, 100);
	}
	
	FNObjectSnapshot BeforeSnapshot;
private:
	
	UFUNCTION()
	void Execute()
	{
		RemoveFromRoot();
		
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
	}
};