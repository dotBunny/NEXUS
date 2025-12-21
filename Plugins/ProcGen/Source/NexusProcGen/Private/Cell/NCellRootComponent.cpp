// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellRootComponent.h"
#include "NProcGenRegistry.h"
#include "NLevelUtils.h"
#include "NProcGenDebugDraw.h"
#include "NProcGenUtils.h"
#include "Math/NVectorUtils.h"
#include "LevelInstance/LevelInstanceActor.h"

FRotator UNCellRootComponent::GetOffsetRotator() const
{
	if (LevelInstance != nullptr)
	{
		return LevelInstance->GetActorRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UNCellRootComponent::GetOffsetLocation() const
{
	if (LevelInstance != nullptr)
	{
		return LevelInstance->GetActorLocation();
	}
	return FVector::ZeroVector;
}

FBox UNCellRootComponent::GetOffsetBounds() const
{
	if (LevelInstance != nullptr)
	{
		return FNProcGenUtils::CreateRotatedBox(
			this->Details.Bounds, 
			LevelInstance->GetActorRotation(), 
			LevelInstance->GetActorLocation());
	}
	return this->Details.Bounds;
}

void UNCellRootComponent::OnRegister()
{
	// Is this part of a level instance?
	if (ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this))
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}
	FNProcGenRegistry::RegisterCellRootComponent(this);
	Super::OnRegister();
}

void UNCellRootComponent::OnUnregister()
{
	FNProcGenRegistry::UnregisterCellRootComponent(this);
	Super::OnUnregister();
}

ANCellActor* UNCellRootComponent::GetNCellActor() const
{
	return Cast<ANCellActor>(GetOwner());
}

void UNCellRootComponent::Reset()
{
	Details = FNCellRootDetails();
}

void UNCellRootComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, const uint8 DrawVoxelMode) const
{
	// We need a version that has zero reliance on the EdMode tool
	const FBox RotatedBounds = GetOffsetBounds();
	const TArray<FVector> RotatedVertices = FNVectorUtils::RotateAndOffsetPoints(this->Details.Hull.Vertices, GetOffsetRotator(), GetOffsetLocation());
	
	DrawDebugPDI(PDI, RotatedBounds, FLinearColor::Red, RotatedVertices, FLinearColor::Blue, DrawVoxelMode);
}

void UNCellRootComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI,  const FBox& WorldBoundsBox, const FLinearColor& BoundsColor,  const TArray<FVector>& WorldHullVertices, const FLinearColor& HullColor, const uint8 DrawVoxelMode) const
{
	// Bounds
	DrawWireBox(PDI, WorldBoundsBox, BoundsColor, SDPG_World);
	
	// Hull
	if (WorldHullVertices.Num() > 0)
	{
		FNProcGenDebugDraw::DrawDashedRawMesh(PDI, this->Details.Hull, WorldHullVertices, HullColor, 2, SDPG_World);
	}
	
	// Voxel
	if (DrawVoxelMode == 1)
	{
		FNProcGenDebugDraw::DrawVoxelDataGrid(PDI, this->Details.VoxelData, GetOffsetLocation(), GetOffsetRotator());
	}
	else if (DrawVoxelMode == 2)
	{
		FNProcGenDebugDraw::DrawVoxelDataPoints(PDI, this->Details.VoxelData, GetOffsetLocation(),GetOffsetRotator());
	}
}


