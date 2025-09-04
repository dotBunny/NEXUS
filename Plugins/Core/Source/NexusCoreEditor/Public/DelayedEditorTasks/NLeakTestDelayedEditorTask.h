// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NCoreEditorMinimal.h"
#include "NCoreSettings.h"
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
		
		DelayedMechanism->Start(UNCoreSettings::Get()->LeakCheckTime, 100);
	}
	
private:
	FNObjectSnapshot BeforeSnapshot;
	
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
			N_LOG(Log, TEXT("[FNEditorCommands::OnToolsLeakCheck] Adds detected! COMPARE written to %s."), *DumpFilePath);
		}
		else
		{
			NE_LOG(Log, TEXT("[FNEditorCommands::OnToolsLeakCheck] %s"), *Diff.ToString());
		}
		Release();
	}
};