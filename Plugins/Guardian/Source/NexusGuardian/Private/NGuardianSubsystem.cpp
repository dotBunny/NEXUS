// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianSubsystem.h"

#include "NGuardianMinimal.h"
#include "NGuardianSettings.h"
#include "Async/Async.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NObjectSnapshot.h"
#include "Developer/NObjectSnapshotUtils.h"

void UNGuardianSubsystem::SetBaseline()
{
	BaselineTimerHandle.Invalidate();

	const UNGuardianSettings* Settings = UNGuardianSettings::Get();
	if (Settings == nullptr)
	{
		UE_LOG(LogNexusGuardian, Error, TEXT("Guardian settings was null; unable to set baseline."));
		return;
	}

	if (Settings->ObjectCountWarningThreshold >= Settings->ObjectCountSnapshotThreshold ||
		Settings->ObjectCountSnapshotThreshold >= Settings->ObjectCountCompareThreshold)
	{
		UE_LOG(LogNexusGuardian, Error, TEXT("Guardian settings values must increase between thresholds (warning < snapshot < compare); unable to set baseline."));
		return;
	}

	bPassedObjectCountWarningThreshold = false;
	bPassedObjectCountSnapshotThreshold = false;
	bPassedObjectCountCompareThreshold = false;
	CapturedSnapshot.Reset();

	BaseObjectCount = FNDeveloperUtils::GetCurrentObjectCount();

	// Prevent Overflow
	const int64 ResolvedWarning  = static_cast<int64>(BaseObjectCount) + Settings->ObjectCountWarningThreshold;
	const int64 ResolvedSnapshot = static_cast<int64>(BaseObjectCount) + Settings->ObjectCountSnapshotThreshold;
	const int64 ResolvedCompare  = static_cast<int64>(BaseObjectCount) + Settings->ObjectCountCompareThreshold;
	ObjectCountWarningThreshold  = static_cast<int32>(FMath::Clamp<int64>(ResolvedWarning,  1, MAX_int32));
	ObjectCountSnapshotThreshold = static_cast<int32>(FMath::Clamp<int64>(ResolvedSnapshot, 2, MAX_int32 - 1));
	ObjectCountCompareThreshold  = static_cast<int32>(FMath::Clamp<int64>(ResolvedCompare,  3, MAX_int32 - 2));

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
	TickTimer -= DeltaTime;
	if (TickTimer > 0)
	{
		return;
	}
	TickTimer = TickRate;

	LastObjectCount = FNDeveloperUtils::GetCurrentObjectCount();

	if (LastObjectCount < ObjectCountWarningThreshold && bPassedObjectCountWarningThreshold)
	{
		UE_LOG(LogNexusGuardian, Log, TEXT("The last UObject count has dropped below the warning threshold, resetting threshold triggers and releasing any captured snapshot."));
		bPassedObjectCountWarningThreshold = false;
		bPassedObjectCountSnapshotThreshold = false;
		bPassedObjectCountCompareThreshold = false;
		CapturedSnapshot.Reset();
		return;
	}

	if (LastObjectCount >= ObjectCountWarningThreshold && !bPassedObjectCountWarningThreshold)
	{
		UE_LOG(LogNexusGuardian, Warning, TEXT("The UObject count warning threshold has been met with %d/%d objects."), LastObjectCount, ObjectCountWarningThreshold);
		bPassedObjectCountWarningThreshold = true;

		// We want to wait till next tick/frame at this point before we evaluate the next threshold.
		return;
	}

	if (LastObjectCount >= ObjectCountSnapshotThreshold && !bPassedObjectCountSnapshotThreshold)
	{
		UE_LOG(LogNexusGuardian, Error, TEXT("The UObject count snapshot threshold has been met with %d/%d objects."), LastObjectCount, ObjectCountSnapshotThreshold);
		CapturedSnapshot = FNObjectSnapshotUtils::Snapshot();
		if (bShouldOutputSnapshot)
		{
			FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
				FString::Printf(TEXT("%s_%s.txt"), SnapshotPrefix, *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));

			// The thought process here is that the ToReport is actually more expensive than you would think given it is doing more string manipulation/etc.
			// Maybe it's faster to generate it on the main thread and pass the report in, but the idea was to minimize hitch on GameThread.
			Async(EAsyncExecution::TaskGraph,
				[Snapshot = CapturedSnapshot, DumpFilePath = MoveTemp(DumpFilePath)]()
				{
					const TArray<FString> Output = Snapshot.ToReport().GetReportLines(ENReportOutputFormat::PlainText);
					FFileHelper::SaveStringArrayToFile(Output, *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
					UE_LOG(LogNexusGuardian, Error, TEXT("A UObject snapshot has been written to %s."), *DumpFilePath);
				});
		}
		bPassedObjectCountSnapshotThreshold = true;

		// We want to wait till next tick/frame at this point before we evaluate the next threshold.
		return;
	}

	if (LastObjectCount >= ObjectCountCompareThreshold && !bPassedObjectCountCompareThreshold)
	{
		// Notice ahead of the actual capture to give user feedback
		UE_LOG(LogNexusGuardian, Error, TEXT("The UObject count compare threshold has been met with %d/%d objects."), LastObjectCount, ObjectCountCompareThreshold);

		const FNObjectSnapshot CompareSnapshot = FNObjectSnapshotUtils::Snapshot();
		FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(CapturedSnapshot, CompareSnapshot, false);
		CapturedSnapshot.Reset();

		FString DumpFilePath = FPaths::Combine(FPaths::ProjectLogDir(),
			FString::Printf(TEXT("%s_%s.txt"), ComparePrefix, *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		Async(EAsyncExecution::TaskGraph,
			[Diff = MoveTemp(Diff), DumpFilePath = MoveTemp(DumpFilePath)]()
			{
				FFileHelper::SaveStringArrayToFile(Diff.ToReport().GetReportLines(ENReportOutputFormat::PlainText), *DumpFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
				UE_LOG(LogNexusGuardian, Error, TEXT("A UObject comparison written to %s."), *DumpFilePath);
			});
		bPassedObjectCountCompareThreshold = true;

		// We are at the end of the block so we don't need to return, but again this concept is to move the next threshold eval to the next tick/frame.
	}
}

void UNGuardianSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const UNGuardianSettings* Settings = UNGuardianSettings::Get();
	if (Settings == nullptr)
	{
		UE_LOG(LogNexusGuardian, Error, TEXT("Guardian settings was null; unable to auto baseline."));
		return;
	}
	TickRate = Settings->TickRate;
	TickTimer = TickRate;

	// Spin up doing a baseline after the delay
	if (Settings->bAutoBaseline)
	{
		InWorld.GetTimerManager().SetTimer(BaselineTimerHandle,
			FTimerDelegate::CreateUObject(this, &UNGuardianSubsystem::SetBaseline),
			Settings->AutoBaselineDelay,
			false);
	}
}

void UNGuardianSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	if (BaselineTimerHandle.IsValid())
	{
		InWorld.GetTimerManager().ClearTimer(BaselineTimerHandle);
	}
	Super::OnWorldEndPlay(InWorld);
}
