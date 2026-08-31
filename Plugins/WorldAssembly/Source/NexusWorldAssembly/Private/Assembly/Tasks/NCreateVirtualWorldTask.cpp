// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateVirtualWorldTask.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionCacheActor.h"
#include "NWorldCollisionFingerprint.h"
#include "NWorldCollisionPool.h"
#include "NWorldCollisionSourceKey.h"
#include "Engine/World.h"
#include "Organ/NOrganComponent.h"
#include "Types/NRawMeshFactory.h"

bool FNCreateVirtualWorldTask::TryCaptureFromCache(TSet<uint64>& OutResolvedKeys, TArray<FBoxSphereBounds>& OutMissedBounds,
	bool& bOutMissedUnbounded, bool& bOutAnyMissed, bool& bOutLandscapeResolved) const
{
	OutResolvedKeys.Reset();
	OutMissedBounds.Reset();
	bOutMissedUnbounded = false;
	bOutAnyMissed = false;
	bOutLandscapeResolved = false;

	const UWorld* World = VirtualWorldContextPtr->InputWorld;
	if (World == nullptr) return false;

	// An operation with no organs has nothing to consult a per-organ cache with. Test harnesses build these, and so
	// does any caller that drives the task graph directly, so it falls back rather than failing.
	if (VirtualWorldContextPtr->InputOrgans.IsEmpty()) return false;

	const UNWorldAssemblySettings* AssemblySettings = UNWorldAssemblySettings::Get();
	if (!AssemblySettings->CollisionCacheSettings.bUseCache) return false;

	TArray<UNOrganComponent*> Organs;
	Organs.Reserve(VirtualWorldContextPtr->InputOrgans.Num());
	for (const TObjectPtr<UNOrganComponent>& Organ : VirtualWorldContextPtr->InputOrgans)
	{
		Organs.Add(Organ);
	}

	// Shared with the editor's collision preview rather than implemented here. Both have to make the same three
	// judgements about a pool, and a second copy of them is how the two would come to disagree about what an
	// assembly collides with.
	FNWorldCollisionBaker::FPooledCollision Pooled = FNWorldCollisionBaker::ResolvePooled(World,
		VirtualWorldContextPtr->WorldCollisionSettings, Organs,
		AssemblySettings->CollisionCacheSettings.bValidateCache, true);

	if (!Pooled.bPoolUsable) return false;

	VirtualWorldContextPtr->CachedWorldCollisionMeshes.Append(MoveTemp(Pooled.Meshes));
	OutResolvedKeys = MoveTemp(Pooled.ResolvedKeys);
	bOutLandscapeResolved = Pooled.bLandscapeResolved;

	for (const UNOrganComponent* Organ : Pooled.MissedOrgans)
	{
		bOutAnyMissed = true;

		TArray<FBoxSphereBounds> OrganBounds;
		if (FNWorldCollisionBaker::GetOrganBounds(Organ, OrganBounds) == FNWorldCollisionBaker::EOrganBoundsKind::Unbounded)
		{
			bOutMissedUnbounded = true;
		}
		else
		{
			OutMissedBounds.Append(OrganBounds);
		}
	}

	return true;
}

void FNCreateVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextStart)

	TSet<uint64> ResolvedKeys;
	TArray<FBoxSphereBounds> MissedBounds;
	bool bMissedUnbounded = false;
	bool bAnyMissed = false;
	bool bLandscapeResolved = false;

	const bool bUsedCache = TryCaptureFromCache(ResolvedKeys, MissedBounds, bMissedUnbounded, bAnyMissed,
		bLandscapeResolved);

	// Nothing left to gather: every organ's cache answered, and so did the landscape section.
	if (bUsedCache && !bAnyMissed && bLandscapeResolved)
	{
		N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextFinish)
		return;
	}

	// Guarded on there actually being geometry left to gather. Without this, a run whose organs all resolved but whose
	// landscape went stale would fall through with an empty missed-bounds array — which every gather reads as "the
	// whole level" — and re-extract the entire world to fetch a terrain it could have sampled on its own.
	if (!bUsedCache || bAnyMissed)
	{
		// Collect the world AActors that we need to care about
		const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(VirtualWorldContextPtr->InputWorld,
			CreateWorldActorFilterSettings(VirtualWorldContextPtr->WorldCollisionSettings));

		// With no usable cache this is the whole operation's region, exactly as before. With a partly usable one it is
		// the union of just the organs that missed, so a level where one organ moved does not re-gather for the rest.
		static const TArray<FBoxSphereBounds> Unbounded;
		const TArray<FBoxSphereBounds>& GatherBounds = bUsedCache
			? (bMissedUnbounded ? Unbounded : MissedBounds)
			: VirtualWorldContextPtr->InputBounds;

		// Gather simple-collision meshes from every primitive in the target world, restricted
		// to actors whose bounds fall inside one of the input organs' volume bounds.
		if (!bUsedCache)
		{
			FNRawMeshFactory::FromActorsInBounds(WorldActors, GatherBounds,
				VirtualWorldContextPtr->WorldCollisionMeshes,
				VirtualWorldContextPtr->WorldCollisionTransforms); // We'll bake the meshes off thread in the process phase
		}
		else
		{
			// Provenance is only needed when some geometry came from the cache, because it is the only way to tell
			// which of the gathered meshes the cache already supplied. Emitting it otherwise is an allocation per mesh
			// for nothing.
			TArray<FNRawMesh> Meshes;
			TArray<FTransform> Transforms;
			TArray<FNRawMeshSource> Sources;
			FNRawMeshFactory::FromActorsInBounds(WorldActors, GatherBounds, Meshes, Transforms, &Sources);

			const bool bHaveSources = Sources.Num() == Meshes.Num();
			for (int32 i = 0; i < Meshes.Num(); ++i)
			{
				// An element with no durable identity cannot have been cached, so it is always kept — dropping it
				// would be the one mistake here that removes geometry rather than merely duplicating it.
				const uint64 Key = bHaveSources ? FNWorldCollisionSourceKey::FromSource(Sources[i]) : 0;
				if (Key != 0 && ResolvedKeys.Contains(Key)) continue;

				VirtualWorldContextPtr->WorldCollisionMeshes.Add(MoveTemp(Meshes[i]));
				VirtualWorldContextPtr->WorldCollisionTransforms.Add(Transforms[i]);
			}
		}
	}

	if (!bLandscapeResolved)
	{
		SampleLandscape();
	}

	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextFinish)
}

void FNCreateVirtualWorldTask::SampleLandscape() const
{
	const FNWorldAssemblyWorldCollisionSettings& CollisionSettings = VirtualWorldContextPtr->WorldCollisionSettings;
	if (!CollisionSettings.bIncludeLandscapes) return;

	const UWorld* World = VirtualWorldContextPtr->InputWorld;

	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World,
		CreateWorldActorFilterSettings(CollisionSettings));

	// Mesh terrain comes through the gather above like any other geometry, or not at all when the filter above refused
	// it. Landscape is neither: the factory skips landscape primitives because their collision is a heightfield behind
	// no UBodySetup, so without this an assembly sees no ground and routes cells straight through it. Sampling has to
	// happen here rather than in the process phase because it traces the live physics scene, which is game-thread only
	// — the same reason this task is.
	//
	// Deliberately unbounded, where the geometry gather is restricted to the organs' bounds: the organ volumes describe
	// where cells may go, not what they must clear on the way, and an assembly reads this world well outside them.
	FNRawMeshFactory::FromLandscapesInBounds(WorldActors, {},
		CollisionSettings.LandscapeSampleSpacing,
		VirtualWorldContextPtr->WorldCollisionMeshes,
		VirtualWorldContextPtr->WorldCollisionTransforms);
}
