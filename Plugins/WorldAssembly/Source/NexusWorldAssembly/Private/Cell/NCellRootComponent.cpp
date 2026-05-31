// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellRootComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "NLevelUtils.h"
#include "NWorldAssemblyDebugDraw.h"
#include "NWorldAssemblyUtils.h"
#include "Math/NVectorUtils.h"
#include "LevelInstance/LevelInstanceActor.h"

FRotator UNCellRootComponent::GetOffsetRotator() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UNCellRootComponent::GetOffsetLocation() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorLocation();
	}
	return FVector::ZeroVector;
}

FBox UNCellRootComponent::GetOffsetBounds() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return FNWorldAssemblyUtils::CreateRotatedBox(
			this->Details.Bounds,
			Instance->GetActorRotation(),
			Instance->GetActorLocation());
	}
	return this->Details.Bounds;
}

void UNCellRootComponent::OnRegister()
{
	// Is this part of a level instance?
	ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this);
	if (Interface != nullptr)
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}
	FNWorldAssemblyRegistry::RegisterCellRootComponent(this);
	Super::OnRegister();
}

void UNCellRootComponent::OnUnregister()
{
	FNWorldAssemblyRegistry::UnregisterCellRootComponent(this);
	Super::OnUnregister();
}

ANCellActor* UNCellRootComponent::GetNCellActor() const
{
	return Cast<ANCellActor>(GetOwner());
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
		FNWorldAssemblyDebugDraw::DrawDashedRawMesh(PDI, this->Details.Hull, WorldHullVertices, HullColor, 2, SDPG_World);
	}
	
	// Voxel
	if (DrawVoxelMode == 1)
	{
		FNWorldAssemblyDebugDraw::DrawVoxelDataGrid(PDI, this->Details.VoxelData, GetOffsetLocation(), GetOffsetRotator());
	}
	else if (DrawVoxelMode == 2)
	{
		FNWorldAssemblyDebugDraw::DrawVoxelDataPoints(PDI, this->Details.VoxelData, GetOffsetLocation(),GetOffsetRotator());
	}
}


