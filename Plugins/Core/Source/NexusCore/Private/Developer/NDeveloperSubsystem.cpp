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
	bPassedObjectCountCaptureThreshold = false;
	bPassedObjectCountCompareThreshold = false;
	CaptureSnapshot.Reset();
	
	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();

	ObjectCountWarningThreshold = BaseObjectCount + Settings->DeveloperObjectCountWarningThreshold;
	ObjectCountCaptureThreshold = BaseObjectCount + Settings->DeveloperObjectCountCaptureThreshold;
	ObjectCountCompareThreshold = BaseObjectCount + Settings->DeveloperObjectCountCompareThreshold;
	
	N_LOG(Log, TEXT("[NDeveloperSubsystem::SetBaseline] Watching %i objects and counting: Warning @ %i (+%i) / Capture @ %i (+%i) / Compare @ %i (+%i)"),
		BaseObjectCount,
		ObjectCountWarningThreshold, Settings->DeveloperObjectCountWarningThreshold,
		ObjectCountCaptureThreshold, Settings->DeveloperObjectCountCaptureThreshold,
		ObjectCountCompareThreshold, Settings->DeveloperObjectCountCompareThreshold);
}

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
		N_LOG(Warning, TEXT("[NDeveloperSubsystem::Tick] Object count WARNING threshold met with %d objects."), ObjectCount);
		bPassedObjectCountWarningThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCaptureThreshold && !bPassedObjectCountCaptureThreshold)
	{
		CaptureSnapshot = FNObjectSnapshotUtils::Snapshot();

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_Snapshot_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(CaptureSnapshot.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);

		N_LOG(Error, TEXT("[NDeveloperSubsystem::Tick] Object count CAPTURE threshold met with %d objects, capture written to %s."), ObjectCount, *DumpFilePath);
		bPassedObjectCountCaptureThreshold = true;
		return;
	}
	
	if (ObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		
		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CaptureSnapshot, CompareSnapshot, false);

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("NEXUS_Compare_%s.txt"),*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		FFileHelper::SaveStringToFile(Diff.ToDetailedString(), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);

		N_LOG(Error, TEXT("[NDeveloperSubsystem::Tick] Object count COMPARE threshold met with %d objects, compare written to %s."), ObjectCount, *DumpFilePath);
		bPassedObjectCountCompareThreshold = true;
	}
}

void UNDeveloperSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	SetBaseline();
}
