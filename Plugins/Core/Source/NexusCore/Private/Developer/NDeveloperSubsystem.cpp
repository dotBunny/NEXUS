// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NDeveloperSubsystem.h"

#include "NCoreMinimal.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"

void UNDeveloperSubsystem::SetBaseline()
{
	const UNCoreSettings* Settings = UNCoreSettings::Get();

	bPassedObjectCountWarningThreshold = false;
	bPassedObjectCountSnapshotThreshold = false;
	bPassedObjectCountCompareThreshold = false;
	CaptureSnapshot.Reset();
	
	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();

	ObjectCountWarningThreshold = BaseObjectCount + Settings->DeveloperObjectCountWarningThreshold;
	ObjectCountSnapshotThreshold = BaseObjectCount + Settings->DeveloperObjectCountSnapshotThreshold;
	ObjectCountCompareThreshold = BaseObjectCount + Settings->DeveloperObjectCountCompareThreshold;
	bShouldOutputSnapshot = Settings->bDeveloperObjectCountCaptureOutput;
	
	N_LOG(Log, TEXT("[UNDeveloperSubsystem::SetBaseline] Watching %i objects and counting: Warning @ %i (+%i) / Capture @ %i (+%i) / Compare @ %i (+%i)"),
		BaseObjectCount,
		ObjectCountWarningThreshold, Settings->DeveloperObjectCountWarningThreshold,
		ObjectCountSnapshotThreshold, Settings->DeveloperObjectCountSnapshotThreshold,
		ObjectCountCompareThreshold, Settings->DeveloperObjectCountCompareThreshold);

	bBaselineSet = true;
}

void UNDeveloperSubsystem::Tick(float DeltaTime)
{
	const int ObjectCount = FNDeveloperUtils::GetCurrentObjectCount();
	
	if (ObjectCount < ObjectCountWarningThreshold && bPassedObjectCountWarningThreshold)
	{
		bPassedObjectCountWarningThreshold = false;
		bPassedObjectCountSnapshotThreshold = false;
		bPassedObjectCountCompareThreshold = false;
		CaptureSnapshot.Reset();
		return;
	}
	
	if (ObjectCount >= ObjectCountWarningThreshold && !bPassedObjectCountWarningThreshold)
	{
		N_LOG(Warning, TEXT("[UNDeveloperSubsystem::Tick] Object count WARNING threshold met with %d objects."), ObjectCount);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountSnapshotThreshold && !bPassedObjectCountSnapshotThreshold)
	{
		N_LOG(Error, TEXT("[UNDeveloperSubsystem::Tick] Object count SNAPSHOT threshold met with %d objects."), ObjectCount);
		CaptureSnapshot = FNObjectSnapshotUtils::Snapshot();
		if (bShouldOutputSnapshot)
		{
			const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
				FString::Printf(TEXT("NEXUS_Snapshot_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
			FFileHelper::SaveStringToFile(CaptureSnapshot.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
			
			N_LOG(Error, TEXT("[UNDeveloperSubsystem::Tick] SNAPSHOT written to %s."), *DumpFilePath);
		}
		bPassedObjectCountSnapshotThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		// Notice ahead of the actual capture to give user feedback
		N_LOG(Error, TEXT("[UNDeveloperSubsystem::Tick] Object count COMPARE threshold met with %d objects."), ObjectCount);
		
		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CaptureSnapshot, CompareSnapshot, false);

		const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
		FString::Printf(TEXT("NEXUS_Compare_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
		
		N_LOG(Error, TEXT("[UNDeveloperSubsystem::Tick] COMPARE written to %s."), *DumpFilePath);
		bPassedObjectCountCompareThreshold = true;
	}
}

void UNDeveloperSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	SetBaseline();
}
