// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianSubsystem.h"

#include "NCoreMinimal.h"
#include "NGuardianMinimal.h"
#include "NGuardianSettings.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"

void UNGuardianSubsystem::SetBaseline()
{
	const UNGuardianSettings* Settings = UNGuardianSettings::Get();

	bPassedObjectCountWarningThreshold = false;
	bPassedObjectCountSnapshotThreshold = false;
	bPassedObjectCountCompareThreshold = false;
	CaptureSnapshot.Reset();
	
	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();

	ObjectCountWarningThreshold = BaseObjectCount + Settings->ObjectCountWarningThreshold;
	ObjectCountSnapshotThreshold = BaseObjectCount + Settings->ObjectCountSnapshotThreshold;
	ObjectCountCompareThreshold = BaseObjectCount + Settings->ObjectCountCompareThreshold;
	bShouldOutputSnapshot = Settings->bObjectCountCaptureOutput;
	
	UE_LOG(LogNexusGuardian, Log, TEXT("Watching UObjects(%i). Warning @ %i (+%i) / Capture @ %i (+%i) / Compare @ %i (+%i)."),
		BaseObjectCount,
		ObjectCountWarningThreshold, Settings->ObjectCountWarningThreshold,
		ObjectCountSnapshotThreshold, Settings->ObjectCountSnapshotThreshold,
		ObjectCountCompareThreshold, Settings->ObjectCountCompareThreshold);

	bBaselineSet = true;
}

void UNGuardianSubsystem::Tick(float DeltaTime)
{
	LastObjectCount = FNDeveloperUtils::GetCurrentObjectCount();
	
	if (LastObjectCount < ObjectCountWarningThreshold && bPassedObjectCountWarningThreshold)
	{
		bPassedObjectCountWarningThreshold = false;
		bPassedObjectCountSnapshotThreshold = false;
		bPassedObjectCountCompareThreshold = false;
		CaptureSnapshot.Reset();
		return;
	}
	
	if (LastObjectCount >= ObjectCountWarningThreshold && !bPassedObjectCountWarningThreshold)
	{
		UE_LOG(LogNexusGuardian, Warning, TEXT("The UObject count warning threshold has been met met with %d/%d objects."), LastObjectCount, ObjectCountWarningThreshold);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (LastObjectCount >= ObjectCountSnapshotThreshold && !bPassedObjectCountSnapshotThreshold)
	{
		UE_LOG(LogNexusGuardian, Error, TEXT("The UObject count snapshot threshold has been met with %d/%d objects."), LastObjectCount, ObjectCountSnapshotThreshold);
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
	
	if (LastObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		// Notice ahead of the actual capture to give user feedback
		UE_LOG(LogNexusGuardian, Error, TEXT("Object count compare threshold met with %d objects."), LastObjectCount);
		
		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CaptureSnapshot, CompareSnapshot, false);

		const FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
		FString::Printf(TEXT("NEXUS_Compare_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
		
		UE_LOG(LogNexusGuardian, Error, TEXT("Object count comparison written to %s."), *DumpFilePath);
		bPassedObjectCountCompareThreshold = true;
	}
}

void UNGuardianSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	SetBaseline();
}