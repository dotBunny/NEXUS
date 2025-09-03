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
	
	if (ObjectCount < ObjectCountWarningThreshold && bPassedObjectCountWarningThreshold)
	{
		bPassedObjectCountWarningThreshold = false;
		bPassedObjectCountCaptureThreshold = false;
		bPassedObjectCountCompareThreshold = false;
		CaptureSnapshot.Reset();
		return;
	}
	
	if (ObjectCount >= ObjectCountWarningThreshold && !bPassedObjectCountWarningThreshold)
	{
		N_LOG(Warning, TEXT("[NDeveloperSubsystem::Tick] Object count warning threshold exceeded: %d objects"), ObjectCount);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCaptureThreshold && !bPassedObjectCountCaptureThreshold)
	{
		CaptureSnapshot = FNObjectSnapshotUtils::Snapshot();

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_Snapshot_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(CaptureSnapshot.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);

		N_LOG(Error, TEXT("[NDeveloperSubsystem::Tick] Object count capture threshold exceeded with %d objects, capture output at: "), ObjectCount, *DumpFilePath);
		bPassedObjectCountCaptureThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
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

void UNDeveloperSubsystem::PostInitialize()
{
	const UNCoreSettings* Settings = UNCoreSettings::Get();

	ObjectCountWarningThreshold = Settings->DeveloperObjectCountWarningThreshold;
	ObjectCountCaptureThreshold = Settings->DeveloperObjectCountCaptureThreshold;
	ObjectCountCompareThreshold = Settings->DeveloperObjectCountCompareThreshold;
	
	Super::PostInitialize();
}

void UNDeveloperSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();
	const int32 WarningCount = BaseObjectCount + ObjectCountWarningThreshold;
	const int32 CaptureCount = BaseObjectCount + ObjectCountCaptureThreshold;
	const int32 CompareCount = BaseObjectCount + ObjectCountCompareThreshold;
	
	N_LOG(Log, TEXT("[NDeveloperSubsystem::OnWorldBeginPlay] Reporting for duty! Watching %i objects and counting - Warning @ %i (+%i) / Capture @ %i (+%i) / Compare @ %i (+%i)"),
		BaseObjectCount,
		WarningCount, ObjectCountWarningThreshold,
		CaptureCount, ObjectCountCaptureThreshold,
		CompareCount, ObjectCountCompareThreshold);

	// Cache our updated thresholds
	ObjectCountWarningThreshold = WarningCount;
	ObjectCountCaptureThreshold = CaptureCount;
	ObjectCountCompareThreshold = CompareCount;
}
