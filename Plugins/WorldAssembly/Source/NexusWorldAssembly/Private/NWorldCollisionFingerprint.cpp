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

namespace NEXUS::WorldAssembly::Fingerprint
{
	/**
	 * Indices of FNWorldCollisionFingerprint::FLayers, in the order each ingredient joins the fold.
	 * @note Ordered cheapest-and-most-stable first, so the first layer that differs between two runs is also the
	 *       most fundamental thing that changed.
	 */
	constexpr int32 LayerIdentity = 0;
	constexpr int32 LayerBodies = 1;
	constexpr int32 LayerTags = 2;
	constexpr int32 LayerPlacement = 3;
	constexpr int32 LayerTransforms = 4;
	constexpr int32 LayerCount = 5;
}

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

	/**
	 * @return true when Component carries any of Tags.
	 * @param Component Primitive to test.
	 * @param Tags Tags to look for; an empty list matches nothing.
	 * @note Mirrors the test FNRawMeshFactory::FromActorsInBounds makes when it decides to skip a primitive. The two
	 *       have to answer alike, or the fingerprint would guard geometry the gather does not emit.
	 */
	bool HasAnyComponentTag(const UPrimitiveComponent* Component, const TArray<FName>& Tags)
	{
		for (const FName& Tag : Tags)
		{
			if (Component->ComponentHasTag(Tag))
			{
				return true;
			}
		}
		return false;
	}
}

uint64 FNWorldCollisionFingerprint::Compute(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings, int32* OutContributors)
{
	if (World == nullptr) return 0;

	// Only what is loaded. TActorIterator walks the world's live actors, so in a World Partition level this covers
	// whatever regions and data layers are up at the moment of the call — which is not necessarily what was up when
	// the cache was baked. OutContributors is what makes that visible to a caller reporting a mismatch.
	const TArray<AActor*> Actors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	return ComputeForActors(Actors, Bounds, Settings, OutContributors);
}

uint64 FNWorldCollisionFingerprint::ComputeForActors(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings, int32* OutContributors)
{
	// Folded with a wrapping add rather than an XOR: XOR cancels a pair of identical contributions, and two actors
	// that are genuinely indistinguishable — same name is impossible, but a hash collision is not — would erase each
	// other instead of accumulating.
	uint64 Accumulator = 0;
	int32 Contributors = 0;

	// Resolved once for the whole walk: it is the same list for every actor, and it is what lets each of them report
	// whether the gather would exclude any of its components.
	const TArray<FName> ComponentIgnoreTags = FNCreateVirtualWorldTask::CreateWorldComponentIgnoreTags(Settings);

	for (const AActor* Actor : Actors)
	{
		if (!IsValid(Actor)) continue;
		if (!OverlapsBounds(Actor, Bounds)) continue;

		Accumulator += FNWorldCollisionSourceKey::Mix(HashActor(Actor, ComponentIgnoreTags));
		Contributors++;
	}

	if (OutContributors != nullptr)
	{
		*OutContributors = Contributors;
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

uint64 FNWorldCollisionFingerprint::HashActor(const AActor* Actor, const TArray<FName>& ComponentIgnoreTags,
	FLayers* OutLayers)
{
	if (!IsValid(Actor)) return 0;

	const uint64 ActorIdentity = FNWorldCollisionSourceKey::FromActor(Actor);

	// Four folds of the same walk, each covering one more ingredient than the last, so that a caller diagnosing a
	// rejected cache can attribute it rather than guess. Only the last is the fingerprint; the rest are computed
	// solely when someone asks for the breakdown, which is why FirstLayer starts at the end by default.
	uint64 Layers[NEXUS::WorldAssembly::Fingerprint::LayerCount];
	for (uint64& Layer : Layers)
	{
		Layer = ActorIdentity;
	}
	const int32 FirstLayer = OutLayers != nullptr ? 0 : NEXUS::WorldAssembly::Fingerprint::LayerTransforms;

	int32 PrimitiveCount = 0;
	int32 TotalInstances = 0;

	// Folds Value into the layer it first belongs to and every layer above it, which is what makes each layer a
	// cumulative prefix of the next.
	auto Fold = [&Layers, FirstLayer](const int32 FromLayer, const uint64 Value)
	{
		for (int32 i = FMath::Max(FromLayer, FirstLayer); i < NEXUS::WorldAssembly::Fingerprint::LayerCount; ++i)
		{
			Layers[i] = FNWorldCollisionSourceKey::Combine(Layers[i], Value);
		}
	};

	// Folds into one layer alone, for the two readings of the placement data that are siblings rather than a prefix
	// and its extension — see FLayers::Placement.
	auto FoldInto = [&Layers, FirstLayer](const int32 Layer, const uint64 Value)
	{
		if (Layer < FirstLayer) return;
		Layers[Layer] = FNWorldCollisionSourceKey::Combine(Layers[Layer], Value);
	};

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

		PrimitiveCount++;

		Fold(NEXUS::WorldAssembly::Fingerprint::LayerIdentity,
			FNWorldCollisionSourceKey::HashName(ActorPrimitive->GetFName()));

		// Which cooked collision this is. Serialized with its owning asset, so it survives to a packaged build and
		// changes when the asset's collision is re-authored or the component is pointed at a different mesh.
		// A body built at runtime (a procedural mesh) gets a fresh GUID each session and so correctly never matches.
		Fold(NEXUS::WorldAssembly::Fingerprint::LayerBodies,
			FNWorldCollisionSourceKey::HashGuid(Body->BodySetupGuid));
		Fold(NEXUS::WorldAssembly::Fingerprint::LayerBodies, static_cast<uint64>(Body->GetCollisionTraceFlag()));
		Fold(NEXUS::WorldAssembly::Fingerprint::LayerBodies, static_cast<uint64>(ActorPrimitive->GetCollisionEnabled()));

		// Whether the gather would exclude this component — one bit — and deliberately not the tags themselves.
		//
		// Hashing the whole tag list was the obvious way to notice a component being tagged out, and it was wrong: it
		// made the fingerprint sensitive to every tag anything writes, and generators rewrite component tags for
		// their own bookkeeping on load. PCG stamps its spawned components with its own marker and the source
		// component's name, so a level holding generated content re-hashed differently every session and its cache
		// could never validate. Only the exclusion decision changes what a bake emits, so only the decision is hashed.
		//
		// A change to the ignore list itself is caught separately, by HashSettings.
		if (HasAnyComponentTag(ActorPrimitive, ComponentIgnoreTags))
		{
			// Excluded, and so it contributes that fact and nothing else — the gather emits no geometry for it, and
			// hashing placement the bake never stored would guard the cache against something it does not contain.
			// Without this the exclusion half-worked: geometry left the bake but went on invalidating the cache every
			// time it moved, which is the stability that is most of the reason to tag it out in the first place.
			Fold(NEXUS::WorldAssembly::Fingerprint::LayerTags, 1);
			continue;
		}
		Fold(NEXUS::WorldAssembly::Fingerprint::LayerTags, 0);

		if (const UInstancedStaticMeshComponent* InstanceStaticMesh = Cast<UInstancedStaticMeshComponent>(ActorPrimitive))
		{
			const int32 InstanceCount = InstanceStaticMesh->GetInstanceCount();
			TotalInstances += InstanceCount;
			Fold(NEXUS::WorldAssembly::Fingerprint::LayerPlacement, static_cast<uint64>(InstanceCount));

			// Every instance twice: once into the running sum, which cannot see the order they arrive in, and once
			// into the sequence, which can. Instances that were reordered rather than moved agree on the first and
			// disagree on the second, which is the only way to tell those two apart.
			uint64 UnorderedPlacement = 0;
			for (int32 i = 0; i < InstanceCount; ++i)
			{
				FTransform InstanceWorld;
				InstanceStaticMesh->GetInstanceTransform(i, InstanceWorld, true);

				const uint64 InstanceHash = FNWorldCollisionSourceKey::HashTransform(InstanceWorld);
				UnorderedPlacement += FNWorldCollisionSourceKey::Mix(InstanceHash);
				FoldInto(NEXUS::WorldAssembly::Fingerprint::LayerTransforms, InstanceHash);
			}
			FoldInto(NEXUS::WorldAssembly::Fingerprint::LayerPlacement, UnorderedPlacement);
			continue;
		}

		// A component has one transform, so the two readings cannot differ; both take it.
		const uint64 ComponentHash = FNWorldCollisionSourceKey::HashTransform(ActorPrimitive->GetComponentTransform());
		FoldInto(NEXUS::WorldAssembly::Fingerprint::LayerPlacement, ComponentHash);
		FoldInto(NEXUS::WorldAssembly::Fingerprint::LayerTransforms, ComponentHash);
	}

	if (OutLayers != nullptr)
	{
		OutLayers->Identity = Layers[NEXUS::WorldAssembly::Fingerprint::LayerIdentity];
		OutLayers->Bodies = Layers[NEXUS::WorldAssembly::Fingerprint::LayerBodies];
		OutLayers->Tags = Layers[NEXUS::WorldAssembly::Fingerprint::LayerTags];
		OutLayers->Placement = Layers[NEXUS::WorldAssembly::Fingerprint::LayerPlacement];
		OutLayers->Transforms = Layers[NEXUS::WorldAssembly::Fingerprint::LayerTransforms];
		OutLayers->Primitives = PrimitiveCount;
		OutLayers->Instances = TotalInstances;
	}

	return Layers[NEXUS::WorldAssembly::Fingerprint::LayerTransforms];
}

void FNWorldCollisionFingerprint::LogLayers(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
	const FNWorldAssemblyWorldCollisionSettings& Settings)
{
	if (World == nullptr) return;

	const TArray<AActor*> Actors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(Settings));

	// Folded exactly as ComputeForActors folds its own, so each layer is directly comparable with a fingerprint that
	// stopped at the same ingredient.
	FLayers Totals;
	int32 Contributors = 0;

	const TArray<FName> ComponentIgnoreTags = FNCreateVirtualWorldTask::CreateWorldComponentIgnoreTags(Settings);

	for (const AActor* Actor : Actors)
	{
		if (!IsValid(Actor)) continue;
		if (!OverlapsBounds(Actor, Bounds)) continue;

		FLayers ActorLayers;
		HashActor(Actor, ComponentIgnoreTags, &ActorLayers);

		Totals.Identity += FNWorldCollisionSourceKey::Mix(ActorLayers.Identity);
		Totals.Bodies += FNWorldCollisionSourceKey::Mix(ActorLayers.Bodies);
		Totals.Tags += FNWorldCollisionSourceKey::Mix(ActorLayers.Tags);
		Totals.Placement += FNWorldCollisionSourceKey::Mix(ActorLayers.Placement);
		Totals.Transforms += FNWorldCollisionSourceKey::Mix(ActorLayers.Transforms);
		Totals.Primitives += ActorLayers.Primitives;
		Totals.Instances += ActorLayers.Instances;
		Contributors++;

		// One line per contributing actor, so two captures can be diffed and the differing lines name the actors
		// responsible rather than merely proving that someone is. Keyed on GetName, which is unique within a level
		// and saved, so the same actor lines up across sessions.
		UE_LOG(LogNexusWorldAssembly, Verbose, TEXT("    '%s' placement %llu (%d primitives, %d instances)"),
			*Actor->GetName(), ActorLayers.Placement, ActorLayers.Primitives, ActorLayers.Instances);
	}

	UE_LOG(LogNexusWorldAssembly, Warning,
		TEXT("  layers over %d actors (%d primitives, %d instances): identity %llu, +bodies %llu, +tags %llu, +placement %llu, +transforms %llu"),
		Contributors, Totals.Primitives, Totals.Instances,
		Totals.Identity, Totals.Bodies, Totals.Tags, Totals.Placement, Totals.Transforms);
	UE_LOG(LogNexusWorldAssembly, Verbose,
		TEXT("  (per-actor placement above, at Verbose; diff two captures to name what moved)"));
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
