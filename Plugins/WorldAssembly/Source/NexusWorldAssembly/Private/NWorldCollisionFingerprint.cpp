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

	// Order-independent over the tag list: the same set of ignore tags entered in a different order describes the
	// same filter, and re-ordering an array in the details panel should not invalidate a level's caches.
	uint64 TagAccumulator = 0;
	for (const FName& Tag : Settings.ActorIgnoreTags)
	{
		TagAccumulator += FNWorldCollisionSourceKey::Mix(FNWorldCollisionSourceKey::HashName(Tag));
	}
	Hash = FNWorldCollisionSourceKey::Combine(Hash, TagAccumulator);
	Hash = FNWorldCollisionSourceKey::Combine(Hash, static_cast<uint64>(Settings.ActorIgnoreTags.Num()));

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
