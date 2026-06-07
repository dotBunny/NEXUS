// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellActor.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellLevelInstance.h"

#if WITH_EDITOR
void ANCellActor::PostEditMove(bool bFinished)
{
	// Do not allow the cell to be moved, rotated, or scaled in the editor; always snap back to origin.
	UE_LOG(LogNexusWorldAssembly, Verbose, TEXT("Resetting cell location/rotation/scale as it cannot be moved from origin."));
	CellRoot->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator::ZeroRotator);
	CellRoot->SetWorldScale3D(FVector::OneVector);

	Super::PostEditMove(bFinished);
}

bool ANCellActor::CanDeleteSelectedActor(FText& OutReason) const
{
	static FText NoDeleteReason = FText::FromString("Unable to manually delete NCellActor. Please use the NCell menu to remove it.");
	OutReason = NoDeleteReason;
	return false;	
}

void ANCellActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	if (CellRoot == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("No linked UNCellRootComponent found for the ANCellActor, attempting to find one on the AActor."));
		CellRoot = GetComponentByClass<UNCellRootComponent>();
		if (CellRoot == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to find a UNCellRootComponent for ANCellActor when attempting to find one."));
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
#endif // WITH_EDITOR

void ANCellActor::InitializeFromProxy(ANCellLevelInstance* LevelInstance)
{
	bSpawnedFromProxy = true;
	
	// Disable all actors flagged for editor only
	for (auto Actor : AuthorTimeActors)
	{
		Actor->Destroy(true, false);
	}

	// Code-callback
	OnInitializedFromProxy.ExecuteIfBound(LevelInstance);
	
	// Level-based callback
	for (const auto& Actor : InitializeCallbackActors)
	{
		INCellInitialized::Execute_OnInitializedFromProxy(Actor, LevelInstance);
	}
}

void ANCellActor::CalculateBounds()
{
	CellRoot->Modify();
	
	CellRoot->Details.Bounds = FNWorldAssemblyUtils::CalculatePlayableBounds(GetLevel(), CellRoot->Details.BoundsSettings);

	const FVector UnitSize = UNWorldAssemblySettings::Get()->VoxelSize;

	CellRoot->Details.UnitBounds = FBox(
				FNVectorUtils::GetFurthestGridIntersection(CellRoot->Details.Bounds.Min, UnitSize),
				FNVectorUtils::GetFurthestGridIntersection(CellRoot->Details.Bounds.Max, UnitSize));
	
	const FVector UnitBoundsSize = CellRoot->Details.UnitBounds.GetSize();
	CellRoot->Details.UnitSize = FVector(
		FMath::RoundToInt(UnitBoundsSize.X),
		FMath::RoundToInt(UnitBoundsSize.Y),
		FMath::RoundToInt(UnitBoundsSize.Z));
	
	SetActorDirty();
}

void ANCellActor::SplitHullEdge(const int32 IndexA, const int32 IndexB, const bool bUpdateDerivedData)
{
	CellRoot->Modify();
	CellRoot->Details.Hull.SplitEdge(IndexA, IndexB);
	
	if (bUpdateDerivedData)
	{
		for (const auto Pair : CellJunctions)
		{
			Pair.Value->UpdateHullDerivedData(CellRoot);
		}
	}
	
	SetActorDirty();
}

void ANCellActor::CalculateHull(const bool bUpdateDerivedData)
{
	CellRoot->Modify();
	CellRoot->Details.Hull = FNWorldAssemblyUtils::CalculateConvexHull(GetLevel(), CellRoot->Details.HullSettings);
	
	if (bUpdateDerivedData)
	{
		for (const auto Pair : CellJunctions)
		{
			Pair.Value->UpdateHullDerivedData(CellRoot);
		}
	}
	
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
	CellRoot->Details.VoxelData = FNWorldAssemblyUtils::CalculateVoxelData(GetLevel(), CellRoot->Details.VoxelSettings);
	SetActorDirty();
}