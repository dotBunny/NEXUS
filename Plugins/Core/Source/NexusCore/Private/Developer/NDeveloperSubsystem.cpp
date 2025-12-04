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
	
	UE_LOG(LogNexusCore, Log, TEXT("Watching UObjects(%i). Warning @ %i (+%i) / Capture @ %i (+%i) / Compare @ %i (+%i)."),
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
		UE_LOG(LogNexusCore, Warning, TEXT("The UObject count warning threshold has been met met with %d/%d objects."), ObjectCount, ObjectCountWarningThreshold);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountSnapshotThreshold && !bPassedObjectCountSnapshotThreshold)
	{
		UE_LOG(LogNexusCore, Error, TEXT("The UObject count snapshot threshold has been met with %d/%d objects."), ObjectCount, ObjectCountSnapshotThreshold);
		CaptureSnapshot = FNObjectSnapshotUtils::Snapshot();
		if (bShouldOutputSnapshot)
		{
			const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
				FString::Printf(TEXT("NEXUS_Snapshot_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
			FFileHelper::SaveStringToFile(CaptureSnapshot.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
			
			UE_LOG(LogNexusCore, Error, TEXT("A UObject snapshot has been written to %s."), *DumpFilePath);
		}
		bPassedObjectCountSnapshotThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		// Notice ahead of the actual capture to give user feedback
		UE_LOG(LogNexusCore, Error, TEXT("Object count compare threshold met with %d objects."), ObjectCount);
		
		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CaptureSnapshot, CompareSnapshot, false);

		const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
		FString::Printf(TEXT("NEXUS_Compare_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
		
		UE_LOG(LogNexusCore, Error, TEXT("Object count comparison written to %s."), *DumpFilePath);
		bPassedObjectCountCompareThreshold = true;
	}
}

void UNDeveloperSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	SetBaseline();
}
