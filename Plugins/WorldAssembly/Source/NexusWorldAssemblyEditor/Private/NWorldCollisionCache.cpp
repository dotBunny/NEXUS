// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionCache.h"

#include "NActorUtils.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Types/NRawMeshFactory.h"
#include "Types/NRawMeshUtils.h"
#include "UObject/UObjectGlobals.h"

FNRawMesh FNWorldCollisionCache::CachedMesh;
TWeakObjectPtr<const UWorld> FNWorldCollisionCache::CachedWorld;
bool FNWorldCollisionCache::bDirty = true;
bool FNWorldCollisionCache::bHooksRegistered = false;

const FNRawMesh& FNWorldCollisionCache::Get(const UWorld* World)
{
	EnsureInvalidationHooks();

	if (World == nullptr)
	{
		CachedMesh = FNRawMesh();
		CachedWorld = nullptr;
		bDirty = true;
		return CachedMesh;
	}

	// Rebuild when invalidated by a world change, or when queried for a different world than we last cached.
	if (bDirty || CachedWorld.Get() != World)
	{
		CachedMesh = Build(World, {});
		CachedWorld = World;
		bDirty = false;
	}

	return CachedMesh;
}

FNRawMesh FNWorldCollisionCache::Build(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds, TArray<AActor*>* OutSourceActors)
{
	FNRawMesh MergedMesh;
	if (World == nullptr)
	{
		return MergedMesh;
	}

	// Same world-geometry definition assembly uses: filtered actors' simple collision, gathered within Bounds.
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World, FNCreateVirtualWorldTask::CreateWorldActorFilterSettings());
	if (OutSourceActors != nullptr)
	{
		*OutSourceActors = WorldActors;
	}

	TArray<FNRawMesh> CollisionMeshes;
	TArray<FTransform> CollisionTransforms;
	FNRawMeshFactory::FromActorsInBounds(WorldActors, Bounds, CollisionMeshes, CollisionTransforms);

	// Merge every emitted mesh into one origin-anchored (world-space) mesh, mirroring the collision visualizer path.
	const FTransform MergedTransform = FTransform::Identity;
	for (int32 i = 0; i < CollisionTransforms.Num(); i++)
	{
		FNRawMeshUtils::CombineMesh(MergedTransform, MergedMesh, CollisionTransforms[i], CollisionMeshes[i]);
	}

	return MergedMesh;
}

void FNWorldCollisionCache::Invalidate()
{
	bDirty = true;
}

void FNWorldCollisionCache::EnsureInvalidationHooks()
{
	if (bHooksRegistered)
	{
		return;
	}
	bHooksRegistered = true;

	// Geometry changes that should rebuild the cache. Mirrors the ed mode's collision-visualizer refresh triggers.
	// Bindings live for the editor session (the cache is a process-lifetime static), so handles aren't tracked.
	if (GEngine != nullptr)
	{
		GEngine->OnLevelActorAdded().AddLambda([](AActor*) { Invalidate(); });
		GEngine->OnLevelActorDeleted().AddLambda([](AActor*) { Invalidate(); });
	}
	if (GEditor != nullptr)
	{
		GEditor->OnEndObjectMovement().AddLambda([](UObject&) { Invalidate(); });
	}
	FEditorDelegates::PostUndoRedo.AddLambda([] { Invalidate(); });

	// Skip the continuous mid-edit stream (slider scrubs, gizmo drags); rebuild on the finalizing change instead.
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](UObject*, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			Invalidate();
		}
	});
}
