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

void UNCellRootComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, const uint8 DrawVoxelMode, const FLinearColor& BoundsColor, const FLinearColor& HullColor,
	const bool bDrawBounds, const bool bDrawHull) const
{
	// We need a version that has zero reliance on the EdMode tool
	const FBox RotatedBounds = GetOffsetBounds();

	// Transformed only when it is going to be drawn. This is per-vertex work whose sole consumer is the hull draw
	// below, and it runs for every cell in the level on every frame the overlay is up.
	const TArray<FVector> RotatedVertices = bDrawHull
		? FNVectorUtils::RotateAndOffsetPoints(this->Details.Hull.Vertices, GetOffsetRotator(), GetOffsetLocation())
		: TArray<FVector>();

	DrawDebugPDI(PDI, RotatedBounds, BoundsColor, RotatedVertices, HullColor, DrawVoxelMode, bDrawBounds, bDrawHull);
}

void UNCellRootComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FBox& WorldBoundsBox, const FLinearColor& BoundsColor,
	const TArray<FVector>& WorldHullVertices, const FLinearColor& HullColor, const uint8 DrawVoxelMode,
	const bool bDrawBounds, const bool bDrawHull) const
{
	// Bounds
	if (bDrawBounds)
	{
		DrawWireBox(PDI, WorldBoundsBox, BoundsColor, SDPG_World);
	}

	// Hull
	if (bDrawHull && WorldHullVertices.Num() > 0)
	{
		FNWorldAssemblyDebugDraw::DrawRawMesh(PDI, this->Details.Hull, WorldHullVertices, HullColor, SDPG_World);
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


