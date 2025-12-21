// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellActor.h"

#include "NProcGenMinimal.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"

#if WITH_EDITOR
void ANCellActor::PostEditMove(bool bFinished)
{
	// Do not allow the cell to be moved in the editor
	if (GetActorLocation() != FVector::ZeroVector)
	{
		UE_LOG(LogNexusProcGen, Verbose, TEXT("Resetting cell location/rotation/scale as it cannot be moved from origin."));
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

void ANCellActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	if (CellRoot == nullptr)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("No linked UNCellRootComponent found for the ANCellActor, attempting to find one on the AActor."));
		CellRoot = GetComponentByClass<UNCellRootComponent>();
		if (CellRoot == nullptr)
		{
			UE_LOG(LogNexusProcGen, Error, TEXT("Unable to find a UNCellRootComponent for ANCellActor when attempting to find one."));
		}
	}
}

bool ANCellActor::HasDifferencesFromSidecar() const
{
	// If we don't have a side-car, we are naturally dirty.
	if (Sidecar == nullptr) 
	{
		return true;
	}
	
	if (CellRoot == nullptr) return false;
	
	// Check Cell Root First
	if (!CellRoot->Details.IsEqual(Sidecar->Root))
	{
		return true;
	}
	
	// Junctions Check
	if (Sidecar->Junctions.Num() != CellJunctions.Num())
	{
		return true;
	}
	for (auto Pair : CellJunctions)
	{
		if (!Sidecar->Junctions.Contains(Pair.Key)) return true;
		if (!Sidecar->Junctions[Pair.Key].IsEqual(Pair.Value.Get()->Details)) return true;
	}
				
	return false;
}

void ANCellActor::CalculateBounds()
{
	CellRoot->Modify();
	
	CellRoot->Details.Bounds = FNProcGenUtils::CalculatePlayableBounds(GetLevel(), CellRoot->Details.BoundsSettings);

	const FVector UnitSize = UNProcGenSettings::Get()->UnitSize;

	CellRoot->Details.UnitBounds = FBox(
				FNVectorUtils::GetFurthestGridIntersection(CellRoot->Details.Bounds.Min, UnitSize),
				FNVectorUtils::GetFurthestGridIntersection(CellRoot->Details.Bounds.Max, UnitSize));
	
	
	CellRoot->Details.UnitSize = FVector(
		FMath::RoundToInt(FMath::Abs(CellRoot->Details.UnitBounds.Min.X) + FMath::Abs(CellRoot->Details.UnitBounds.Max.X)),
	FMath::RoundToInt(FMath::Abs(CellRoot->Details.UnitBounds.Min.Y) + FMath::Abs(CellRoot->Details.UnitBounds.Max.Y)),
	FMath::RoundToInt(FMath::Abs(CellRoot->Details.UnitBounds.Min.Z) + FMath::Abs(CellRoot->Details.UnitBounds.Max.Z)));
	
	SetActorDirty();
}

void ANCellActor::CalculateHull()
{
	CellRoot->Modify();
	CellRoot->Details.Hull = FNProcGenUtils::CalculateConvexHull(GetLevel(), CellRoot->Details.HullSettings);
	SetActorDirty();
}

void ANCellActor::CalculateVoxelData()
{
	if (!CellRoot->Details.VoxelSettings.bUseVoxelData)
	{
		if (CellRoot->Details.VoxelData.GetCount() != 0)
		{
			CellRoot->Modify();
			CellRoot->Details.VoxelData = FNCellVoxelData();
			SetActorDirty();
		}
		return;
	}
	
	CellRoot->Modify();
	CellRoot->Details.VoxelData = FNProcGenUtils::CalculateVoxelData(GetLevel(), CellRoot->Details.VoxelSettings);
	SetActorDirty();
}