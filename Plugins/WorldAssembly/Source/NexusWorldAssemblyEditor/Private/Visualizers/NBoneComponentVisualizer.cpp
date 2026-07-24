// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NBoneComponentVisualizer.h"

#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionCache.h"
#include "Math/NMeshBVH.h"
#include "Organ/NBoneComponent.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"

uint32 FNBoneComponentVisualizer::CachedGeneration = 0;
TMap<TWeakObjectPtr<const UNBoneComponent>, FNBoneComponentVisualizer::FCachedPenetration> FNBoneComponentVisualizer::PenetrationCache;

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

	// Penetration is a pure function of (world-collision mesh, bone transform, socket size) — none of which change
	// frame to frame in the common idle case — so it is memoized rather than re-swept on every viewport redraw.
	float WorldPenetration = 0.f;
	if (Settings != nullptr)
	{
		WorldPenetration = GetCachedWorldPenetration(BoneComponent, Settings);
	}

	// We are always going to draw this
	BoneComponent->DrawDebugPDI(PDI, FNWorldAssemblyEdMode::GetCachedBoneValidColor(),  FNWorldAssemblyEdMode::GetCachedBoneInvalidColor(),
		true,  true, Settings, WorldPenetration);
}

float FNBoneComponentVisualizer::GetCachedWorldPenetration(const UNBoneComponent* BoneComponent, const UNWorldAssemblySettings* Settings)
{
	const UWorld* World = BoneComponent->GetWorld();

	// Kick a background rebuild if world collision changed. This never blocks: the gather + merge + BVH build happen off
	// the draw (see FNWorldCollisionCache async path), so a viewport redraw is always cheap even right after an edit.
	FNWorldCollisionCache::RequestAsyncRefresh(World);

	// The memo clears only when NEW results publish (ResultsGeneration), not on invalidation — so during a rebuild we
	// keep drawing the last-known value instead of recomputing against soon-to-be-replaced data.
	const uint32 Generation = FNWorldCollisionCache::GetResultsGeneration();
	if (Generation != CachedGeneration)
	{
		PenetrationCache.Reset();
		CachedGeneration = Generation;
	}

	// Per-bone inputs that change the result independently of world geometry.
	const FTransform CurrentTransform = BoneComponent->GetComponentTransform();
	const FIntVector2 CurrentSocketSize = BoneComponent->SocketSize;
	const FVector2D CurrentSettingSocketSize = Settings->SocketSize;

	const TWeakObjectPtr<const UNBoneComponent> Key(BoneComponent);
	if (const FCachedPenetration* Existing = PenetrationCache.Find(Key))
	{
		if (Existing->KeySocketSize == CurrentSocketSize
			&& Existing->KeySettingSocketSize == CurrentSettingSocketSize
			&& Existing->KeyTransform.Equals(CurrentTransform))
		{
			return Existing->Penetration;
		}
	}

	// Miss — sample the socket corners against the most recently published world-collision mesh + BVH. When nothing has
	// been published yet (first build still running) both are null and we report 0 until results land and bump the
	// generation above.
	float WorldPenetration = 0.f;
	const TSharedPtr<const FNMeshBVH> WorldCollisionBVH = FNWorldCollisionCache::GetPublishedBVH();
	const TSharedPtr<const FNRawMesh> WorldCollisionMesh = FNWorldCollisionCache::GetPublishedMesh();
	if (WorldCollisionBVH.IsValid() && WorldCollisionMesh.IsValid() && WorldCollisionMesh->Loops.Num() > 0)
	{
		const TArray<FVector> CornerPoints = BoneComponent->GetWorldCornerPoints(Settings->SocketSize);

		// The merged world-collision mesh is non-convex in any real level; sample it through the BVH, which reproduces
		// FNRawMeshUtils::ComputePointDepthInsideNonConvex exactly but visits only the geometry near each corner. The
		// convex / non-triangle degenerate cases (trivial single-body test levels) keep the original exact path, whose
		// convex face-plane metric the BVH does not replicate.
		if (!FNWorldCollisionCache::IsPublishedMeshConvex() && !FNWorldCollisionCache::PublishedMeshHasNonTris())
		{
			for (const FVector& Corner : CornerPoints)
			{
				WorldPenetration = FMath::Max(WorldPenetration, WorldCollisionBVH->GetPointDepth(Corner));
			}
		}
		else
		{
			for (const FVector& Corner : CornerPoints)
			{
				const float Depth = FNRawMeshUtils::GetIntersectDepth(*WorldCollisionMesh, FVector::ZeroVector, FRotator::ZeroRotator, Corner);
				WorldPenetration = FMath::Max(WorldPenetration, Depth);
			}
		}
	}

	FCachedPenetration& Entry = PenetrationCache.FindOrAdd(Key);
	Entry.KeyTransform = CurrentTransform;
	Entry.KeySocketSize = CurrentSocketSize;
	Entry.KeySettingSocketSize = CurrentSettingSocketSize;
	Entry.Penetration = WorldPenetration;
	return WorldPenetration;
}
