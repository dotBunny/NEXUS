// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellActor.h"

#include "NCoreMinimal.h"
#include "NProcGenUtils.h"

#if WITH_EDITOR
void ANCellActor::PostEditMove(bool bFinished)
{
	// Do not allow the cell to be moved in the editor
	if (GetActorLocation() != FVector::ZeroVector)
	{
		N_LOG(Log, TEXT("[ANCellActor::PostEditMove] Resetting cell location/rotation/scale."));
		CellRoot->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator::ZeroRotator);
		CellRoot->SetWorldScale3D(FVector::OneVector);
	}
}

bool ANCellActor::CanDeleteSelectedActor(FText& OutReason) const
{
	static FText NoDeleteReason = FText::FromString("Unable to manually delete NCellActor. Please use the NCell menu to remove it.");
	OutReason = NoDeleteReason;
	return false;	
}

#endif // WITH_EDITOR

void ANCellActor::InitializeFromProxy()
{
	bSpawnedFromProxy = true;

	// Callback
	OnInitializedFromProxy.Broadcast();
}

void ANCellActor::CalculateBounds()
{
	CellRoot->Details.Bounds = FNProcGenUtils::CalculatePlayableBounds(GetLevel(), CellRoot->Details.BoundsSettings);
	SetActorDirty();
}

void ANCellActor::CalculateHull()
{
	CellRoot->Details.Hull = FNProcGenUtils::CalculateConvexHull(GetLevel(), CellRoot->Details.HullSettings);
	SetActorDirty();
}
