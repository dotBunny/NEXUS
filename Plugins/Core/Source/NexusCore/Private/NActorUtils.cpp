// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorUtils.h"
#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Developer/NDebugActor.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Volume.h"
#include "Engine/SCS_Node.h"

namespace NEXUS::Core::Terrain
{
	/**
	 * Class-name prefix shared by every landscape component. Prefix rather than exact match because the render
	 * component, the heightfield collision component and the mesh collision component are all distinct classes.
	 */
	constexpr const TCHAR* LandscapeClassPrefix = TEXT("Landscape");

	/**
	 * Class name of the Mesh Partition component holding a section's cooked terrain collision. Its sibling mesh
	 * components carry only render geometry, so this is the one that identifies a section as terrain.
	 */
	constexpr const TCHAR* MeshPartitionCollisionClassName = TEXT("MeshPartitionCollisionComponent");

	/**
	 * Actor class names of the Mesh Partition sections that hold built terrain geometry — the editor's transient
	 * preview sections, and the compiled sections a cook produces.
	 * @note The interactive section is deliberately absent. It is a working copy of whatever region is being sculpted
	 *       right now, duplicating geometry a preview section already describes, and it changes on every mouse move.
	 */
	constexpr const TCHAR* PreviewSectionClassName = TEXT("PreviewSection");
	constexpr const TCHAR* CompiledSectionClassName = TEXT("CompiledSection");

	/**
	 * Actor class names of the Mesh Partition authoring actors: the partition definition itself, and the modifiers
	 * describing the operations that sculpt it.
	 */
	constexpr const TCHAR* MeshPartitionClassName = TEXT("MeshPartition");
	constexpr const TCHAR* ModifierActorClassName = TEXT("ModifierActor");

	/**
	 * Half-extent below which a primitive is treated as describing nothing.
	 *
	 * The placeholder the engine substitutes for empty geometry is a cube of half-extent KINDA_SMALL_NUMBER +
	 * SMALL_NUMBER, sized that way expressly so it clears the IsNearlyZero() check other systems apply — which is
	 * why FVector::IsNearlyZero cannot be used here and a threshold is needed instead. This one sits two orders of
	 * magnitude above the placeholder and far below any real terrain section, which spans metres.
	 */
	constexpr double MinimumBuiltHalfExtent = 0.01;
}

USceneComponent* FNActorUtils::GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass)
{
	if (ActorClass == nullptr) return nullptr;

	// Walk the (possibly multi-level) Blueprint class chain, returning the first
	// USceneComponent template found on any SCS root node.
	for (UClass* Cursor = *ActorClass; Cursor; Cursor = Cursor->GetSuperClass())
	{
		const UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(Cursor);
		if (BPGC == nullptr) break; // hit the first native class — stop SCS walk

		const USimpleConstructionScript* SCS = BPGC->SimpleConstructionScript;
		if (SCS == nullptr) continue;

		for (const USCS_Node* Node : SCS->GetRootNodes())
		{
			if (Node == nullptr) continue;
			if (USceneComponent* SceneTemplate = Cast<USceneComponent>(Node->ComponentTemplate))
			{
				return SceneTemplate;
			}
		}
	}

	// Native fallback: read the CDO's root (set by the native constructor).
	const AActor* CDO = ActorClass->GetDefaultObject<AActor>();
	return CDO ? CDO->GetRootComponent() : nullptr;
}


TArray<AActor*> FNActorUtils::GetWorldActors(const UWorld* World, const FNWorldActorFilterSettings& Settings)
{
	TArray<AActor*> ReturnActors;
	if (World == nullptr) return ReturnActors;

	for (TActorIterator<AActor> WorldActorIterator(World); WorldActorIterator; ++WorldActorIterator)
	{
		AActor* Actor = *WorldActorIterator;
		if (PassesFilter(Actor, Settings))
		{
			ReturnActors.Add(Actor);
		}
	}

	return ReturnActors;
}

bool FNActorUtils::PassesFilter(const AActor* Actor, const FNWorldActorFilterSettings& Settings)
{
	// Ensure it is not on its way to being destroyed
	if (!Actor || !IsValid(Actor)) return false;

	if (Settings.bIncludePlayerStarts && Actor->IsA<APlayerStart>()) return true;

	// Exclude editor only
	if (Settings.bExcludeEditorOnly && Actor->IsEditorOnly()) return false;

	// Exclude when collision is disabled per setting
	if (Settings.bExcludeNonCollisionEnabledActors && !Actor->GetActorEnableCollision()) return false;

	// Exclude based on Actor Tags
	for (int i = 0; i < Settings.WorldCollisionActorIgnoreTags.Num(); ++i)
	{
		if (Actor->ActorHasTag(Settings.WorldCollisionActorIgnoreTags[i]))
		{
			return false;
		}
	}

	// Exclude by class, ahead of the two checks below that have to walk the actor's components.
	if (Settings.bExcludeVolumes && Actor->IsA<AVolume>()) return false;
	if (Settings.bExcludeDebugActors && Actor->IsA<ANDebugActor>()) return false;
	if (Settings.bExcludeTerrainAuthoring && IsTerrainAuthoringActor(Actor)) return false;

	// Left until last of the built-in checks: these are the only two that inspect the actor's components, so they are
	// only paid by whatever everything above already let through.
	if (Settings.bExcludeMeshTerrains && IsMeshTerrainActor(Actor)) return false;
	if (Settings.bExcludeLandscapes && IsLandscapeActor(Actor)) return false;

	// Exclude because of filter
	if (Settings.ExclusionFunction.IsSet() && !Settings.ExclusionFunction(Actor)) return false;

	return true;
}

bool FNActorUtils::IsLandscapeClassName(const FString& ClassName)
{
	// Prefix, not exact: the render component, the heightfield collision component and the mesh collision component
	// are distinct classes sharing it.
	return ClassName.StartsWith(NEXUS::Core::Terrain::LandscapeClassPrefix);
}

bool FNActorUtils::IsMeshTerrainPrimitiveClassName(const FString& ClassName)
{
	return ClassName == NEXUS::Core::Terrain::MeshPartitionCollisionClassName;
}

bool FNActorUtils::IsTerrainPrimitiveClassName(const FString& ClassName)
{
	return IsLandscapeClassName(ClassName) || IsMeshTerrainPrimitiveClassName(ClassName);
}

bool FNActorUtils::IsTerrainSectionClassName(const FString& ClassName)
{
	return ClassName == NEXUS::Core::Terrain::PreviewSectionClassName
		|| ClassName == NEXUS::Core::Terrain::CompiledSectionClassName;
}

bool FNActorUtils::IsTerrainAuthoringClassName(const FString& ClassName)
{
	return ClassName == NEXUS::Core::Terrain::MeshPartitionClassName
		|| ClassName == NEXUS::Core::Terrain::ModifierActorClassName;
}

bool FNActorUtils::IsTerrainPrimitive(const UPrimitiveComponent* Primitive)
{
	if (Primitive == nullptr) return false;
	return IsTerrainPrimitiveClassName(Primitive->GetClass()->GetName());
}

bool FNActorUtils::IsMeshTerrainActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return false;

	// Section actors are matched on their own class rather than by finding a collision component, because they do not
	// reliably have one: Mesh Partition attaches collision in a pass after the section is spawned, and a definition
	// with collision generation disabled never attaches one at all. Such a section still carries the built terrain on
	// its preview and static-mesh components, so recognizing it only by collision would leave a level's entire floor
	// classified as an anonymous transient actor and dropped.
	if (IsTerrainSectionClassName(Actor->GetClass()->GetName())) return true;

	// Unregistered primitives count. A Mesh Partition section is rebuilt asynchronously, so its collision component
	// can exist while still awaiting registration — treating that window as "not terrain" would let a save-time
	// recalculation drop the actor on a technicality it recovers from a frame later.
	TInlineComponentArray<UPrimitiveComponent*> Primitives(Actor);
	for (const UPrimitiveComponent* Primitive : Primitives)
	{
		if (Primitive == nullptr) continue;
		if (IsMeshTerrainPrimitiveClassName(Primitive->GetClass()->GetName())) return true;
	}

	return false;
}

bool FNActorUtils::IsTerrainActor(const AActor* Actor)
{
	// Either representation. Composed rather than walking the primitives again, so the two halves and the whole
	// cannot drift apart.
	return IsMeshTerrainActor(Actor) || IsLandscapeActor(Actor);
}

bool FNActorUtils::IsTerrainAuthoringActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return false;
	return IsTerrainAuthoringClassName(Actor->GetClass()->GetName());
}

bool FNActorUtils::IsLandscapeActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return false;

	TInlineComponentArray<UPrimitiveComponent*> Primitives(Actor);
	for (const UPrimitiveComponent* Primitive : Primitives)
	{
		if (Primitive == nullptr) continue;
		if (IsLandscapeClassName(Primitive->GetClass()->GetName())) return true;
	}

	return false;
}

bool FNActorUtils::HasBuiltGeometry(const UPrimitiveComponent* Primitive)
{
	if (Primitive == nullptr) return false;

	const FVector Extent = Primitive->Bounds.BoxExtent;
	return Extent.X > NEXUS::Core::Terrain::MinimumBuiltHalfExtent
		|| Extent.Y > NEXUS::Core::Terrain::MinimumBuiltHalfExtent
		|| Extent.Z > NEXUS::Core::Terrain::MinimumBuiltHalfExtent;
}

FBox FNActorUtils::GetBuiltComponentsBoundingBox(const AActor* Actor, const bool bIncludeNonColliding)
{
	FBox Box(ForceInit);
	if (!IsValid(Actor)) return Box;

	// Mirrors AActor::GetComponentsBoundingBox's registration and collision gates so the only behavioral difference
	// is the placeholder rejection. Child-actor components are left out for the same reason that function omits them
	// by default — they belong to an actor of their own.
	TInlineComponentArray<UPrimitiveComponent*> Primitives(Actor);
	for (const UPrimitiveComponent* Primitive : Primitives)
	{
		if (Primitive == nullptr || !Primitive->IsRegistered()) continue;
		if (!bIncludeNonColliding && !Primitive->IsCollisionEnabled()) continue;
		if (!HasBuiltGeometry(Primitive)) continue;

		Box += Primitive->Bounds.GetBox();
	}

	return Box;
}
