// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NBoneComponentVisualizer.h"

#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionCache.h"
#include "Organ/NBoneComponent.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"

void FNBoneComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	// We check that we indeed have an actor component
	const UNBoneComponent* BoneComponent = Cast<UNBoneComponent>(const_cast<UActorComponent*>(Component));
	if (!BoneComponent)
	{
		return;
	}

	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();

	// Measure how deep the socket corners sit inside world collision, sampling the shared (and invalidation-tracked)
	// world-collision mesh rather than re-gathering it here. Mirrors the junction's hull test, just against the world.
	float WorldPenetration = 0.f;
	if (Settings != nullptr)
	{
		const FNRawMesh& WorldCollisionMesh = FNWorldCollisionCache::Get(BoneComponent->GetWorld());
		if (WorldCollisionMesh.Loops.Num() > 0)
		{
			const TArray<FVector> CornerPoints = BoneComponent->GetWorldCornerPoints(Settings->SocketSize);
			for (const FVector& Corner : CornerPoints)
			{
				const float Depth = FNRawMeshUtils::GetIntersectDepth(WorldCollisionMesh, FVector::ZeroVector, FRotator::ZeroRotator, Corner);
				WorldPenetration = FMath::Max(WorldPenetration, Depth);
			}
		}
	}

	// We are always going to draw this
	BoneComponent->DrawDebugPDI(PDI, FNWorldAssemblyEdMode::GetCachedBoneValidColor(),  FNWorldAssemblyEdMode::GetCachedBoneInvalidColor(), true, Settings, WorldPenetration);
}
