// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NBoneComponentVisualizer.h"

#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionPreview.h"
#include "Math/NMeshBVH.h"
#include "Organ/NBoneComponent.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"

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
	BoneComponent->DrawDebugPDI(PDI, FNWorldAssemblyEditorColors::GetBoneValid(), FNWorldAssemblyEditorColors::GetBoneInvalid(),
		true,  true, Settings, WorldPenetration);
}

float FNBoneComponentVisualizer::GetCachedWorldPenetration(const UNBoneComponent* BoneComponent, const UNWorldAssemblySettings* Settings)
{
	const UWorld* World = BoneComponent->GetWorld();

	// Read straight from the level's baked collision, which is the geometry an assembly will actually test against.
	// Nothing is gathered here: a bake does not happen while you work, so the merge and BVH behind this are memoized
	// and only rebuilt when a geometry edit or a bake invalidates them.
	//
	// Notably, moving a bone does not invalidate anything — bones carry no collision and are filtered out of the bake
	// entirely — so the readout stays live through exactly the edit it exists to support.
	const FNWorldCollisionPreview::EState PreviewState = FNWorldCollisionPreview::GetState(World);
	if (PreviewState != FNWorldCollisionPreview::EState::Available)
	{
		// Said out loud rather than silently reading zero, which would look like "this bone is clear". Sampling then
		// continues against the last baked state when there is one — an out-of-date answer the user has been told
		// about beats no answer at all, and beats a confident wrong zero.
		FNWorldCollisionPreview::NotifyUnavailable(BoneComponent->GetWorld(), PreviewState);
	}

	// Bumped whenever the preview could differ from a previous read, so the per-bone memo below refreshes with it.
	const uint32 Generation = FNWorldCollisionPreview::GetGeneration(World);

	// Per-bone inputs that change the result independently of world geometry.
	const FTransform CurrentTransform = BoneComponent->GetComponentTransform();
	const FIntVector2 CurrentSocketSize = BoneComponent->SocketSize;
	const FVector2D CurrentSettingSocketSize = Settings->SocketSize;

	const TWeakObjectPtr<const UNBoneComponent> Key(BoneComponent);
	if (const FCachedPenetration* Existing = PenetrationCache.Find(Key))
	{
		if (Existing->KeyResultsGeneration == Generation
			&& Existing->KeySocketSize == CurrentSocketSize
			&& Existing->KeySettingSocketSize == CurrentSettingSocketSize
			&& Existing->KeyTransform.Equals(CurrentTransform))
		{
			return Existing->Penetration;
		}
	}

	// Miss — we are about to recompute this entry. Misses are rare (a real input or generation change), so use the
	// opportunity to drop entries whose bone has been destroyed; without a whole-map reset this is what keeps the memo
	// from growing across selection changes and across the multiple worlds it serves.
	for (TMap<TWeakObjectPtr<const UNBoneComponent>, FCachedPenetration>::TIterator It(PenetrationCache); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	// Sample the socket corners against this world's most recently published world-collision mesh + BVH. When nothing
	// has been published yet (first build still running) both are null and we report 0 until results land and bump the
	// generation above.
	float WorldPenetration = 0.f;
	const FNRawMesh& WorldCollisionMesh = FNWorldCollisionPreview::GetMesh(World);
	if (WorldCollisionMesh.Loops.Num() > 0)
	{
		const FNMeshBVH& WorldCollisionBVH = FNWorldCollisionPreview::GetBVH(World);
		const TArray<FVector> CornerPoints = BoneComponent->GetWorldCornerPoints(Settings->SocketSize);

		// The merged world-collision mesh is non-convex in any real level; sample it through the BVH, which reproduces
		// FNRawMeshUtils::ComputePointDepthInsideNonConvex exactly but visits only the geometry near each corner. The
		// convex / non-triangle degenerate cases (trivial single-body test levels) keep the original exact path, whose
		// convex face-plane metric the BVH does not replicate.
		if (!WorldCollisionMesh.IsConvex() && !WorldCollisionMesh.HasNonTris())
		{
			for (const FVector& Corner : CornerPoints)
			{
				WorldPenetration = FMath::Max(WorldPenetration, WorldCollisionBVH.GetPointDepth(Corner));
			}
		}
		else
		{
			for (const FVector& Corner : CornerPoints)
			{
				const float Depth = FNRawMeshUtils::GetIntersectDepth(WorldCollisionMesh, FVector::ZeroVector, FRotator::ZeroRotator, Corner);
				WorldPenetration = FMath::Max(WorldPenetration, Depth);
			}
		}
	}

	FCachedPenetration& Entry = PenetrationCache.FindOrAdd(Key);
	Entry.KeyTransform = CurrentTransform;
	Entry.KeySocketSize = CurrentSocketSize;
	Entry.KeySettingSocketSize = CurrentSettingSocketSize;
	Entry.KeyResultsGeneration = Generation;
	Entry.Penetration = WorldPenetration;
	return WorldPenetration;
}
