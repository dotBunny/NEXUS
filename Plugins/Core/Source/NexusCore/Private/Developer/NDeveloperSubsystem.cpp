// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NDeveloperSubsystem.h"

#include "NCoreMinimal.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"

void UNDeveloperSubsystem::Tick(float DeltaTime)
{
	const int ObjectCount = FNDeveloperUtils::GetCurrentObjectCount();
	const int Difference = ObjectCount - BaseObjectCount;

	if (Difference < ObjectCountWarningThreshold && bPassedObjectCountWarningThreshold)
	{
		bPassedObjectCountWarningThreshold = false;
		bPassedObjectCountCaptureThreshold = false;
		bPassedObjectCountCompareThreshold = false;
		CaptureSnapshot.Reset();
		return;
	}
	
	if (Difference >= ObjectCountWarningThreshold && !bPassedObjectCountWarningThreshold)
	{
		N_LOG(Warning, TEXT("[NDeveloperSubsystem::Tick] Object count warning threshold exceeded: %d objects"), ObjectCount);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (Difference >= ObjectCountCaptureThreshold && !bPassedObjectCountCaptureThreshold)
	{
		CaptureSnapshot = FNObjectSnapshotUtils::Snapshot();

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_Snapshot_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(CaptureSnapshot.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);

		N_LOG(Error, TEXT("[NDeveloperSubsystem::Tick] Object count capture threshold exceeded with %d objects, capture output at: "), ObjectCount, *DumpFilePath);
		bPassedObjectCountCaptureThreshold = true;
		return;
	}
	
	if (Difference >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		
		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CaptureSnapshot, CompareSnapshot, true);

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_Compare_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);

		N_LOG(Error, TEXT("[NDeveloperSubsystem::Tick] Object count compare threshold exceeded with %d objects, compare output at: %s"), ObjectCount, *DumpFilePath);
		bPassedObjectCountCompareThreshold = true;
	}
}

void UNDeveloperSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Load Settings
	const UNCoreSettings* Settings = UNCoreSettings::Get();

	ObjectCountWarningThreshold = Settings->DeveloperObjectCountWarningThreshold;
	ObjectCountCaptureThreshold = Settings->DeveloperObjectCountCaptureThreshold;
	ObjectCountCompareThreshold = Settings->DeveloperObjectCountCompareThreshold;
	
	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();
	N_LOG(Log, TEXT("[NDeveloperSubsystem::OnWorldBeginPlay] Reporting for duty! Watching %i objects and counting - Warning @ %i | Capture @ %i | Compare @ %i"),
		BaseObjectCount, ObjectCountWarningThreshold, ObjectCountCaptureThreshold, ObjectCountCompareThreshold);
}
