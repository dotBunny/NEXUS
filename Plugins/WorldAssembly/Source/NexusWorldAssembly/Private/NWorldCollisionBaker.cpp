// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionBaker.h"

#include "NActorUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "NWorldCollisionCacheActor.h"
#include "NWorldCollisionFingerprint.h"
#include "NWorldCollisionPool.h"
#include "NWorldCollisionSourceKey.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Engine/World.h"
#include "GameFramework/Volume.h"
#include "Organ/NOrganComponent.h"
#include "Types/NRawMeshFactory.h"
#include "Misc/ScopedSlowTask.h"
#include "Types/NRawMeshUtils.h"

void FNWorldCollisionBaker::BakeElement(FNRawMesh& Mesh, const FTransform& Transform)
{
	Mesh.ApplyTransform(Transform);

	// The step that used to live only in FNProcessVirtualWorldTask. Every collision test in an assembly — cell
	// placement and junction connection alike — runs against the hull rather than the source geometry, so anything
	// claiming to represent what an assembly sees has to convexify too or it describes a world that is not tested.
	if (!Mesh.IsConvex())
	{
		Mesh = FNRawMeshUtils::ToConvexHull(Mesh);
	}

	// Warmed here, off whatever thread is baking, so the first intersection query does not pay for it.
	Mesh.EnsureCachedFacePlanes();
}

FNWorldCollisionBaker::EOrganBoundsKind FNWorldCollisionBaker::GetOrganBounds(const UNOrganComponent* Organ,
	TArray<FBoxSphereBounds>& OutBounds)
{
	OutBounds.Reset();

	if (Organ == nullptr) return EOrganBoundsKind::None;

	// An unbound organ makes the gather read the whole level — the same thing LockAndPreprocess does when it empties
	// the bounds array on finding one.
	if (Organ->bUnbound) return EOrganBoundsKind::Unbounded;

	// A bounded organ with no volume is skipped rather than promoted to the whole level, matching the warn-and-skip
	// in LockAndPreprocess. Promoting it would make a cached run consider geometry a fresh run never gathered.
	if (!Organ->IsVolumeBased()) return EOrganBoundsKind::None;

	const AVolume* Volume = Organ->GetVolume();
	if (Volume == nullptr) return EOrganBoundsKind::None;

	OutBounds.Add(Volume->GetBounds());
	return EOrganBoundsKind::Bounded;
}

TArray<UNOrganComponent*> FNWorldCollisionBaker::GetWorldOrgans(const UWorld* World)
{
	TArray<UNOrganComponent*> Organs;
	if (World == nullptr) return Organs;

	for (const ULevel* Level : World->GetLevels())
	{
		if (Level == nullptr || Level->IsInstancedLevel()) continue;
		Organs.Append(FNWorldAssemblyUtils::GetOrganComponentsFromLevel(Level));
	}
	return Organs;
}

FNWorldCollisionBaker::FPooledCollision FNWorldCollisionBaker::ResolvePooled(const UWorld* World,
	const FNWorldAssemblyWorldCollisionSettings& Settings, const TArray<UNOrganComponent*>& Organs,
	const bool bValidate, const bool bIncludeLandscape)
{
	FPooledCollision Result;

	// The pool is addressed through organs, so a world without any has nothing to resolve — not even landscape, which
	// only an assembly would be reading.
	if (World == nullptr || Organs.IsEmpty()) return Result;

	// The actor's existence is the signal that a bake has happened; what it found is a separate question. An empty
	// pool is a legitimate result — a level with organs but no world geometry yet bakes to exactly that — and reading
	// it as "never baked" would leave such a level permanently reported as unbaked however often it was baked.
	const ANWorldCollisionCacheActor* CacheActor = ANWorldCollisionCacheActor::Find(World);
	if (CacheActor == nullptr) return Result;

	// Hulls are stored in world space, so a rebased origin misplaces every one of them while changing no actor and
	// passing every fingerprint. This is the one staleness test that has to be made against the world itself.
	if (CacheActor->Pool.BakeOriginLocation != World->OriginLocation)
	{
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("World collision cache was baked at origin %s but the world is now at %s; ignoring it."),
			*CacheActor->Pool.BakeOriginLocation.ToString(), *World->OriginLocation.ToString());
		return Result;
	}

	Result.bPoolUsable = true;

	for (UNOrganComponent* Organ : Organs)
	{
		if (!IsValid(Organ)) continue;

		TArray<FBoxSphereBounds> Bounds;
		const EOrganBoundsKind BoundsKind = GetOrganBounds(Organ, Bounds);

		// Contributes no region, so it contributes no geometry either — from the pool or from a gather.
		if (BoundsKind == EOrganBoundsKind::None) continue;

		bool bHit = false;
		if (Organ->CollisionCache.HasData())
		{
			// Skipping the fingerprint is the "trust what was baked" setting: the cache is taken at its word, which is
			// only right for a world whose geometry is fixed by the time anything reads it.
			const uint64 Current = bValidate
				? FNWorldCollisionFingerprint::Compute(World, Bounds, Settings)
				: Organ->CollisionCache.Fingerprint;

			if (Organ->CollisionCache.IsValidFor(Current))
			{
				// Elements an earlier organ already resolved are skipped rather than resolved twice — overlapping
				// organs, and any two unbounded ones, name much of the same geometry.
				TArray<uint64> Unresolved;
				Unresolved.Reserve(Organ->CollisionCache.SourceKeys.Num());
				for (const uint64 Key : Organ->CollisionCache.SourceKeys)
				{
					if (!Result.ResolvedKeys.Contains(Key))
					{
						Unresolved.Add(Key);
					}
				}

				if (CacheActor->Pool.Resolve(Unresolved, Result.Meshes))
				{
					Result.ResolvedKeys.Append(Unresolved);
					bHit = true;
				}
				else
				{
					// A key the pool no longer holds. Detectable only because organs reference geometry by key; an
					// index list would have silently resolved to whatever now occupies that slot.
					UE_LOG(LogNexusWorldAssembly, Warning,
						TEXT("Organ '%s' references world collision the level's pool no longer holds; gathering fresh for it."),
						*Organ->GetName());
				}
			}
			else
			{
				UE_LOG(LogNexusWorldAssembly, Warning,
					TEXT("Organ '%s' world collision cache is stale (baked %llu, world is %llu); gathering fresh for it."),
					*Organ->GetName(), Organ->CollisionCache.Fingerprint, Current);
			}
		}

		if (!bHit)
		{
			Result.MissedOrgans.Add(Organ);
		}
	}

	if (!bIncludeLandscape) return Result;

	// Nothing to resolve is the same answer as resolved, from the caller's side: either way it has no landscape left
	// to sample for itself.
	if (!Settings.bIncludeLandscapes)
	{
		Result.bLandscapeResolved = true;
		return Result;
	}

	const uint64 CurrentLandscape = FNWorldCollisionFingerprint::ComputeLandscape(World, Settings);
	if (CurrentLandscape != 0 && CurrentLandscape == CacheActor->Pool.LandscapeFingerprint)
	{
		Result.Meshes.Append(CacheActor->Pool.LandscapeMeshes);
		Result.bLandscapeResolved = true;
		return Result;
	}

	if (!CacheActor->Pool.LandscapeMeshes.IsEmpty())
	{
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("Cached landscape collision is stale (baked %llu, world is %llu); sampling fresh."),
			CacheActor->Pool.LandscapeFingerprint, CurrentLandscape);
	}

	return Result;
}

bool FNWorldCollisionBaker::BakeOrgan(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings, FOrganResult& OutResult)
{
	OutResult.Fingerprint = 0;
	OutResult.Keys.Reset();
	OutResult.Meshes.Reset();

	if (World == nullptr) return false;

	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	TArray<FNRawMesh> Meshes;
	TArray<FTransform> Transforms;
	TArray<FNRawMeshSource> Sources;
	FNRawMeshFactory::FromActorsInBounds(WorldActors, Bounds, Meshes, Transforms, &Sources);

	if (Sources.Num() != Meshes.Num())
	{
		// The factory emits one provenance record per mesh; a mismatch means an emit path grew without recording,
		// and pairing the two arrays anyway would key hulls to the wrong geometry. Refuse rather than guess.
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("World collision bake gathered %d meshes against %d provenance records; not caching."),
			Meshes.Num(), Sources.Num());
		return false;
	}

	OutResult.Keys.Reserve(Meshes.Num());
	OutResult.Meshes.Reserve(Meshes.Num());

	TSet<uint64> SeenKeys;
	SeenKeys.Reserve(Meshes.Num());

	for (int32 i = 0; i < Meshes.Num(); ++i)
	{
		const uint64 Key = FNWorldCollisionSourceKey::FromSource(Sources[i]);

		// An element with no durable identity cannot be pooled, and an organ that silently dropped it would resolve
		// to less geometry than a fresh gather — placing cells through something a live run would have avoided.
		if (Key == 0)
		{
			UE_LOG(LogNexusWorldAssembly, Warning,
				TEXT("World collision bake found an element with no stable identity (%s); not caching this organ."),
				Sources[i].Component != nullptr ? *Sources[i].Component->GetPathName() : TEXT("unknown"));
			OutResult.Keys.Reset();
			OutResult.Meshes.Reset();
			return false;
		}

		// Two elements sharing a key would collapse into one pool entry and lose geometry. Identity is built to make
		// this impossible, so hitting it means the key composition is wrong rather than the level being unusual.
		if (SeenKeys.Contains(Key))
		{
			UE_LOG(LogNexusWorldAssembly, Warning,
				TEXT("World collision bake produced a duplicate element key for %s; not caching this organ."),
				Sources[i].Component != nullptr ? *Sources[i].Component->GetPathName() : TEXT("unknown"));
			OutResult.Keys.Reset();
			OutResult.Meshes.Reset();
			return false;
		}
		SeenKeys.Add(Key);

		BakeElement(Meshes[i], Transforms[i]);

		OutResult.Keys.Add(Key);
		OutResult.Meshes.Add(MoveTemp(Meshes[i]));
	}

	// Fingerprinted from the same actor set the gather just read, rather than by walking the world again — so the
	// fingerprint cannot describe a world one edit newer than the geometry stored beside it.
	OutResult.Fingerprint = FNWorldCollisionFingerprint::ComputeForActors(WorldActors, Bounds, Settings);

	return true;
}

bool FNWorldCollisionBaker::BakeLandscape(const UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings,
	FNWorldCollisionPool& Pool)
{
	if (World == nullptr) return false;

	const uint64 Fingerprint = FNWorldCollisionFingerprint::ComputeLandscape(World, Settings);

	// Landscape capture is off, or there is nothing to sample. Clearing rather than leaving the old samples in place
	// matters: they would otherwise keep being fed to every assembly long after the setting was turned off.
	if (Fingerprint == 0)
	{
		const bool bChanged = !Pool.LandscapeMeshes.IsEmpty() || Pool.LandscapeFingerprint != 0;
		Pool.LandscapeMeshes.Reset();
		Pool.LandscapeFingerprint = 0;
		return bChanged;
	}

	// Already sampled from this exact landscape state — and this is the skip worth having, since resampling is the
	// most expensive thing the bake can do.
	if (Fingerprint == Pool.LandscapeFingerprint) return false;

	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	TArray<FNRawMesh> Meshes;
	TArray<FTransform> Transforms;

	// Deliberately unbounded, matching FNCreateVirtualWorldTask: the organ volumes describe where cells may go, not
	// what they must clear on the way, and an assembly reads the ground well outside them.
	FNRawMeshFactory::FromLandscapesInBounds(WorldActors, {}, Settings.LandscapeSampleSpacing, Meshes, Transforms);

	for (int32 i = 0; i < Meshes.Num(); ++i)
	{
		BakeElement(Meshes[i], Transforms[i]);
	}

	Pool.LandscapeMeshes = MoveTemp(Meshes);
	Pool.LandscapeFingerprint = Fingerprint;

	return true;
}

FNWorldCollisionBaker::FNOnWorldCollisionBaked FNWorldCollisionBaker::OnBaked;

#if WITH_EDITOR
FNWorldCollisionBaker::FBakeResult FNWorldCollisionBaker::BakeWorld(UWorld* World,
	const FNWorldAssemblyWorldCollisionSettings& Settings, const bool bForce)
{
	if (World == nullptr) return FBakeResult();

	return BakeOrgans(World, GetWorldOrgans(World), Settings, bForce);
}

FNWorldCollisionBaker::FBakeResult FNWorldCollisionBaker::BakeOrgans(UWorld* World, const TArray<UNOrganComponent*>& Organs,
	const FNWorldAssemblyWorldCollisionSettings& Settings, const bool bForce)
{
	FBakeResult Result;
	if (World == nullptr) return Result;

	ANWorldCollisionCacheActor* CacheActor = ANWorldCollisionCacheActor::FindOrCreate(World);
	if (CacheActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Could not create a world collision cache actor; nothing cached."));
		return Result;
	}

	// One frame per organ plus one for landscape. Worth a dialog because none of this is incremental from the user's
	// side: every organ costs a fingerprint pass at minimum, a re-gather flushes pending static-mesh compilation, and
	// sampling landscape traces the physics scene once per sample — the editor is simply stopped until it finishes.
	FScopedSlowTask BakeTask = FScopedSlowTask(static_cast<float>(Organs.Num() + 1),
		NSLOCTEXT("NexusWorldAssembly", "Task_BakeWorldCollision", "Baking World Collision"));
	BakeTask.MakeDialog(true);

	bool bChanged = false;
	CacheActor->Modify();

	// Hulls are baked in world space, so the pool is only meaningful against the origin it was baked at. Recorded
	// here and checked at consume time, since a rebased origin changes no actor and would pass every fingerprint.
	if (CacheActor->Pool.BakeOriginLocation != World->OriginLocation)
	{
		CacheActor->Pool.BakeOriginLocation = World->OriginLocation;
		bChanged = true;
	}

	for (UNOrganComponent* Organ : Organs)
	{
		if (BakeTask.ShouldCancel())
		{
			// Everything baked so far stands: each organ's stored fingerprint still describes what it was baked
			// against, so a half-finished bake leaves a consistent level rather than a torn one.
			Result.bCancelled = true;
			break;
		}
		BakeTask.EnterProgressFrame(1.0f, FText::Format(
			NSLOCTEXT("NexusWorldAssembly", "Task_BakeWorldCollision_Organ", "Baking world collision for {0}"),
			FText::FromString(IsValid(Organ) ? Organ->GetName() : TEXT("organ"))));

		if (!IsValid(Organ)) continue;

		TArray<FBoxSphereBounds> Bounds;
		const EOrganBoundsKind BoundsKind = GetOrganBounds(Organ, Bounds);

		// An organ that contributes no region has nothing to cache. Clear anything it holds rather than leaving a
		// key list that would resolve geometry a fresh gather would never have read for it.
		if (BoundsKind == EOrganBoundsKind::None)
		{
			if (Organ->CollisionCache.HasData())
			{
				Organ->Modify();
				Organ->CollisionCache.Reset();
				bChanged = true;
			}
			continue;
		}

		// The cheap half: an organ whose world has not changed keeps the cache it has, and the whole bake costs one
		// fingerprint pass. This is what makes baking on every level save affordable.
		const uint64 Fingerprint = FNWorldCollisionFingerprint::Compute(World, Bounds, Settings);
		if (!bForce && Organ->CollisionCache.IsValidFor(Fingerprint))
		{
			continue;
		}

		FOrganResult OrganResult;
		if (!BakeOrgan(World, Bounds, Settings, OrganResult))
		{
			// BakeOrgan has already said why. Clear whatever this organ held rather than leaving a cache that no
			// longer matches the world behind — a stale hit is worse than a miss.
			if (Organ->CollisionCache.HasData())
			{
				Organ->Modify();
				Organ->CollisionCache.Reset();
				bChanged = true;
			}
			continue;
		}

		for (int32 i = 0; i < OrganResult.Keys.Num(); ++i)
		{
			CacheActor->Pool.Add(OrganResult.Keys[i], MoveTemp(OrganResult.Meshes[i]));
		}

		Organ->Modify();
		Organ->CollisionCache.SetBakeResult(OrganResult.Fingerprint, MoveTemp(OrganResult.Keys));

		Result.OrgansBaked++;
		bChanged = true;
	}

	BakeTask.EnterProgressFrame(1.0f,
		NSLOCTEXT("NexusWorldAssembly", "Task_BakeWorldCollision_Landscape", "Sampling landscape collision"));

	if (BakeLandscape(World, Settings, CacheActor->Pool))
	{
		bChanged = true;
	}

	// Orphan collection. The union is taken over every organ in the level rather than the ones just baked, so an
	// organ that was skipped as still-valid keeps its elements — and one that was deleted takes its own with it.
	TSet<uint64> Referenced;
	for (const UNOrganComponent* Organ : GetWorldOrgans(World))
	{
		if (!IsValid(Organ)) continue;
		Referenced.Append(Organ->CollisionCache.SourceKeys);
	}

	if (CacheActor->Pool.RemoveUnreferenced(Referenced) > 0)
	{
		bChanged = true;
	}

	if (bChanged)
	{
		CacheActor->MarkPackageDirty();
	}

	// Announced whether or not anything was written. A bake that changed nothing still settles the question for every
	// organ it looked at, and a view memoized against an earlier, more pessimistic answer should re-ask rather than go
	// on reporting it.
	OnBaked.Broadcast(World);

	Result.bChanged = bChanged;
	return Result;
}
#endif // WITH_EDITOR
