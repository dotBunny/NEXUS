// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionFingerprint.h"

#include "NActorUtils.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionSourceKey.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodySetup.h"

namespace
{
	/**
	 * @return An order-independent summary of a list of names, plus its length.
	 * @param Names Names to summarize.
	 * @note Order-independent because the same set of tags in a different order describes the same thing, whether it
	 *       is an ignore list re-ordered in the details panel or a component's own tags rebuilt by whatever wrote
	 *       them. Neither should invalidate a level's caches. The count is folded in separately so that a list and a
	 *       longer one summing to the same accumulator are still told apart.
	 */
	uint64 HashNameSet(const TArray<FName>& Names)
	{
		uint64 Accumulator = 0;
		for (const FName& Name : Names)
		{
			Accumulator += FNWorldCollisionSourceKey::Mix(FNWorldCollisionSourceKey::HashName(Name));
		}
		return FNWorldCollisionSourceKey::Combine(Accumulator, static_cast<uint64>(Names.Num()));
	}
}

uint64 FNWorldCollisionFingerprint::Compute(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings)
{
	if (World == nullptr) return 0;

	const TArray<AActor*> Actors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	return ComputeForActors(Actors, Bounds, Settings);
}

uint64 FNWorldCollisionFingerprint::ComputeForActors(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings)
{
	// Folded with a wrapping add rather than an XOR: XOR cancels a pair of identical contributions, and two actors
	// that are genuinely indistinguishable — same name is impossible, but a hash collision is not — would erase each
	// other instead of accumulating.
	uint64 Accumulator = 0;
	int32 Contributors = 0;

	for (const AActor* Actor : Actors)
	{
		if (!IsValid(Actor)) continue;
		if (!OverlapsBounds(Actor, Bounds)) continue;

		Accumulator += FNWorldCollisionSourceKey::Mix(HashActor(Actor));
		Contributors++;
	}

	uint64 Fingerprint = FNWorldCollisionSourceKey::Combine(Accumulator, static_cast<uint64>(Contributors));
	Fingerprint = FNWorldCollisionSourceKey::Combine(Fingerprint, HashBounds(Bounds));
	Fingerprint = FNWorldCollisionSourceKey::Combine(Fingerprint, HashSettings(Settings));

	// Zero is the "never fingerprinted" sentinel on a stored cache, so a real fingerprint must not produce it.
	return Fingerprint == 0 ? 1 : Fingerprint;
}

uint64 FNWorldCollisionFingerprint::ComputeLandscape(const UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings)
{
	if (World == nullptr) return 0;
	if (!Settings.bIncludeLandscapes) return 0;

	const TArray<AActor*> Actors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	uint64 Accumulator = 0;
	int32 Contributors = 0;

	for (const AActor* Actor : Actors)
	{
		if (!IsValid(Actor) || !FNActorUtils::IsLandscapeActor(Actor)) continue;

		uint64 Hash = FNWorldCollisionSourceKey::FromActor(Actor);
		Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashTransform(Actor->GetActorTransform()));

		// The collision components' extents stand in for the height data itself, which has no runtime-readable
		// saved identity. Resizing, adding, or removing a landscape component is caught; sculpting within one is not.
		TInlineComponentArray<UPrimitiveComponent*> ActorPrimitives(Actor);
		for (const UPrimitiveComponent* ActorPrimitive : ActorPrimitives)
		{
			if (ActorPrimitive == nullptr || !ActorPrimitive->IsRegistered()) continue;

			const FBoxSphereBounds PrimitiveBounds = ActorPrimitive->Bounds;
			Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashVector(PrimitiveBounds.Origin));
			Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashVector(PrimitiveBounds.BoxExtent));
		}

		Accumulator += FNWorldCollisionSourceKey::Mix(Hash);
		Contributors++;
	}

	// No landscape in the level is a legitimate state, and distinct from "landscape capture is off" above — both
	// produce an empty section, but only one of them should stay valid when a landscape is later added.
	uint64 Fingerprint = FNWorldCollisionSourceKey::Combine(Accumulator, static_cast<uint64>(Contributors));
	Fingerprint = FNWorldCollisionSourceKey::Combine(Fingerprint,
		static_cast<uint64>(FMath::RoundToInt64(Settings.LandscapeSampleSpacing * NEXUS::WorldAssembly::CollisionKey::LocationQuantum)));

	return Fingerprint == 0 ? 1 : Fingerprint;
}

uint64 FNWorldCollisionFingerprint::HashActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return 0;

	uint64 Hash = FNWorldCollisionSourceKey::FromActor(Actor);

	// Non-const because UPrimitiveComponent::GetBodySetup is non-const; nothing here mutates the component.
	TInlineComponentArray<UPrimitiveComponent*> ActorPrimitives(Actor);
	for (UPrimitiveComponent* ActorPrimitive : ActorPrimitives)
	{
		if (ActorPrimitive == nullptr || !ActorPrimitive->IsRegistered()) continue;

		// Landscape primitives are skipped by the geometry gather (their collision is a heightfield behind no
		// UBodySetup), so including them here would make a fingerprint sensitive to something it does not cover.
		// The landscape cache carries its own fingerprint for exactly that reason.
		if (FNActorUtils::IsLandscapeClassName(ActorPrimitive->GetClass()->GetName())) continue;

		const UBodySetup* Body = ActorPrimitive->GetBodySetup();
		if (Body == nullptr) continue;

		Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashName(ActorPrimitive->GetFName()));

		// Which cooked collision this is. Serialized with its owning asset, so it survives to a packaged build and
		// changes when the asset's collision is re-authored or the component is pointed at a different mesh.
		// A body built at runtime (a procedural mesh) gets a fresh GUID each session and so correctly never matches.
		Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashGuid(Body->BodySetupGuid));
		Hash = FNWorldCollisionSourceKey::Combine(Hash, static_cast<uint64>(Body->GetCollisionTraceFlag()));
		Hash = FNWorldCollisionSourceKey::Combine(Hash, static_cast<uint64>(ActorPrimitive->GetCollisionEnabled()));

		// The component's own tags, because one of them may be what excludes it from the gather. Hashed rather than
		// acted on — exactly as GetCollisionEnabled above is, and for the same reason: this summarizes what a gather
		// would see without re-deciding it, so the ignore list does not have to be threaded down here to stay correct.
		// Leaving them out is the one omission that would be silently wrong rather than merely conservative: tagging a
		// component would change the geometry a bake emits without changing the fingerprint guarding it, and the cache
		// would go on reporting itself current while holding the geometry that was just excluded.
		Hash = FNWorldCollisionSourceKey::Combine(Hash, HashNameSet(ActorPrimitive->ComponentTags));

		if (const UInstancedStaticMeshComponent* InstanceStaticMesh = Cast<UInstancedStaticMeshComponent>(ActorPrimitive))
		{
			const int32 InstanceCount = InstanceStaticMesh->GetInstanceCount();
			Hash = FNWorldCollisionSourceKey::Combine(Hash, static_cast<uint64>(InstanceCount));

			for (int32 i = 0; i < InstanceCount; ++i)
			{
				FTransform InstanceWorld;
				InstanceStaticMesh->GetInstanceTransform(i, InstanceWorld, true);
				Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashTransform(InstanceWorld));
			}
			continue;
		}

		Hash = FNWorldCollisionSourceKey::Combine(Hash, FNWorldCollisionSourceKey::HashTransform(ActorPrimitive->GetComponentTransform()));
	}

	return Hash;
}

uint64 FNWorldCollisionFingerprint::HashSettings(const FNWorldAssemblyWorldCollisionSettings& Settings)
{
	uint64 Hash = FNWorldCollisionSourceKey::Mix(Settings.bExcludeNonCollisionEnabledActors ? 1 : 0);
	Hash = FNWorldCollisionSourceKey::Combine(Hash, Settings.bIncludePlayerStarts ? 1 : 0);
	Hash = FNWorldCollisionSourceKey::Combine(Hash, Settings.bIncludeLandscapes ? 1 : 0);
	Hash = FNWorldCollisionSourceKey::Combine(Hash, Settings.bIncludeMeshTerrains ? 1 : 0);
	Hash = FNWorldCollisionSourceKey::Combine(Hash,
		static_cast<uint64>(FMath::RoundToInt64(Settings.LandscapeSampleSpacing * NEXUS::WorldAssembly::CollisionKey::LocationQuantum)));

	Hash = FNWorldCollisionSourceKey::Combine(Hash, HashNameSet(Settings.ActorIgnoreTags));
	Hash = FNWorldCollisionSourceKey::Combine(Hash, HashNameSet(Settings.ComponentIgnoreTags));

	return Hash;
}

bool FNWorldCollisionFingerprint::OverlapsBounds(const AActor* Actor, const TArray<FBoxSphereBounds>& Bounds)
{
	if (Bounds.IsEmpty()) return true;
	if (!IsValid(Actor)) return false;

	FVector ActorOrigin;
	FVector ActorExtent;
	Actor->GetActorBounds(true, ActorOrigin, ActorExtent, true);
	const FBox ActorBox(ActorOrigin - ActorExtent, ActorOrigin + ActorExtent);

	for (const FBoxSphereBounds& Bound : Bounds)
	{
		if (ActorBox.Intersect(Bound.GetBox()))
		{
			return true;
		}
	}
	return false;
}

uint64 FNWorldCollisionFingerprint::HashBounds(const TArray<FBoxSphereBounds>& Bounds)
{
	// Order-independent for the same reason the actor fold is: the bounds array is assembled by walking the
	// operation's generation order, which is stable within a run but not something a cache should depend on.
	uint64 Accumulator = 0;
	for (const FBoxSphereBounds& Bound : Bounds)
	{
		uint64 BoundHash = FNWorldCollisionSourceKey::HashVector(Bound.Origin);
		BoundHash = FNWorldCollisionSourceKey::Combine(BoundHash, FNWorldCollisionSourceKey::HashVector(Bound.BoxExtent));
		Accumulator += FNWorldCollisionSourceKey::Mix(BoundHash);
	}
	return FNWorldCollisionSourceKey::Combine(Accumulator, static_cast<uint64>(Bounds.Num()));
}
