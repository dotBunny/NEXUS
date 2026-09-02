// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellAssemblyData.h"
#include "NCellJunctionConnectorEntry.h"
#include "NCellJunctionDetails.h"
#include "NCellJunctionFillerEntry.h"
#include "NCellLinkDetails.h"
#include "NWorldAssemblySettings.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "NCellJunctionComponent.generated.h"

class ANCellLevelInstance;
class UNCellRootComponent;
class ALevelInstance;
class UNCell;
struct FNRawMesh;

/**
 * Scene component marking a junction on a cell — a connection point that lets one cell attach to another during generation.
 *
 * Carries the junction's shape (FNCellJunctionDetails) and provides derived data (corner points, color) used by
 * the graph builder and debug visualizers to reason about cell connectivity.
 * @see <a href="https://nexus-framework.com/docs/world-assembly/types/junction-component/">UNCellJunctionComponent</a>
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Cell Junction", meta=(BlueprintSpawnableComponent,
	DocsURL="https://nexus-framework.com/docs/world-assembly/types/junction-component"),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSWORLDASSEMBLY_API UNCellJunctionComponent : public USceneComponent
{
	GENERATED_BODY()

	explicit UNCellJunctionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Details()
		, LevelInstance(nullptr)
	{
		PrimaryComponentTick.bCanEverTick = false;
		PrimaryComponentTick.bStartWithTickEnabled = false;
		bAutoActivate = 0;


		Mobility = EComponentMobility::Static;
#if WITH_EDITOR
		TArray<USceneComponent*> ParentComponents;
		this->GetParentComponents(ParentComponents);
		for (USceneComponent* Parent : ParentComponents)
		{
			if (Parent)
			{
				Parent->TransformUpdated.AddUObject(this, &UNCellJunctionComponent::OnTransformUpdated);
			}
		}
		TransformUpdated.AddUObject(this, &UNCellJunctionComponent::OnTransformUpdated);
#endif // WITH_EDITOR
	}

public:
	/** Junction shape, orientation, and flags authored per junction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cell Junction")
	FNCellJunctionDetails Details;

	UPROPERTY(EditInstanceOnly, DisplayName="OnBeginPlay Targets", Category = "Cell Junction",
	meta=(AllowedClasses="/Script/NexusWorldAssembly.NCellJunctionBeginPlay"))
	TArray<TObjectPtr<AActor>> OnBeginPlayTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Disable Filling", Category = "Cell Junction|Fill",
		meta=(ToolTip="Should the junction not be filled regardless of the requirements."))
	bool bDisableFill = false;

	/** Candidate fillers for this junction; one is selected (constraint-gated, then weighted-random) and spawned when the junction is left unconnected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cell Junction|Fill", meta=(TitleProperty="{Actor}", EditCondition="!bDisableFill"))
	TArray<FNCellJunctionFillerEntry> Fillers;

	/**
	 * Candidate connectors for this junction, used when the connector pass pairs it with another cell's junction.
	 *
	 * Takes priority over the owning organ's list and the project-wide default. When a pairing has a list at both
	 * ends, the start end's wins; see FNCellJunctionConnection for which end that is.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cell Junction|Connection", meta=(TitleProperty="{Actor}"))
	TArray<FNCellJunctionConnectorEntry> Connectors;

	/** When true, bypass filler time-slicing and spawn this junction's filler immediately during BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Spawn Filler Immediately", Category = "Cell Junction|Fill",
		meta=(ToolTip="Override timeslicing support and immediately spawn this filler in BeginPlay", EditCondition="!bDisableFill"))
	bool bSpawnFillerImmediately = false;

	/**
	 * Actors shown when this junction ends up filled (that is, unconnected), and hidden when it does not.
	 *
	 * Driven exactly once per junction, from ResolveConnectionState, so the state holds for every junction however
	 * it resolved — including ones that never fill because their requirements allow an empty opening, and ones whose
	 * fill finds no eligible filler.
	 */
	UPROPERTY(EditInstanceOnly, DisplayName="Additional Filled Actors", Category = "Cell Junction|Fill",
		meta=(ToolTip="Any actors that should be enabled when this Junction is filled (not connected), will be disabled otherwise."))
	TArray<TObjectPtr<AActor>> AdditionalFilledActors;

	/**
	 * Actors shown when this junction ends up connected to another, and hidden when it does not.
	 *
	 * Applied after AdditionalFilledActors, so an actor named in both lists takes its state from this one.
	 * @see AdditionalFilledActors for when the pair is driven.
	 */
	UPROPERTY(EditInstanceOnly, DisplayName="Additional Connected Actors", Category = "Cell Junction|Fill",
		meta=(ToolTip="Any actors that should be enabled when this Junction is connected (not filled), will be disabled otherwise. Runs after the AdditionalFilledActors are processed which means it can override those Actor settings."))
	TArray<TObjectPtr<AActor>> AdditionalConnectedActors;

	/** Connection state for this junction, resolved during generation; its bConnected flag gates whether the junction is filled at BeginPlay. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Assembly Operation")
	FNCellLinkDetails LinkDetails;

	//~USceneComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//End USceneComponent

	/** @return The cell level instance this junction was spawned as part of, or nullptr before it is streamed in. */
	ALevelInstance* GetLevelInstance() const { return LevelInstance.Get(); }

	/** @return The rotational offset authored on the junction's details. */
	FRotator GetOffsetRotator() const;

	/** @return The location offset authored on the junction's details. */
	FVector GetOffsetLocation() const;

	/** @return The fill depth magnitude for this junction: the project SocketDepth for the Default* modes, or the authored OverrideFillDepth for the Override* modes. */
	float GetFillDepth() const;

	/**
	 * Signed offset, along the junction's forward axis, at which a filler should anchor its fill volume before extruding
	 * forward by GetFillDepth(). Encodes the directional half of the fill-depth mode without moving the spawn transform.
	 * @return 0 for the forward modes, -GetFillDepth() for the backward modes, and -GetFillDepth()/2 for the centered modes.
	 */
	float GetFillDepthAnchor() const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR

	virtual void PostEditImport() override;

	/** @return The display name of this junction. */
	FString GetJunctionName() const;
	/** Transform-changed callback that keeps the junction's hull-derived data in sync after the component moves. */
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
#endif // WITH_EDITOR

	/**
	 * @return The deepest penetration of any point in CornerPoints into Hull, or 0 when none of them are inside.
	 * @param Hull Cell hull to measure against, in the same space as CornerPoints.
	 * @param CornerPoints Socket corner points to sample.
	 * @note Points outside the hull report a GetIntersectDepth sentinel of -1, which this collapses to 0 so the
	 *       result reads as "no penetration" rather than a negative depth.
	 */
	static float ComputeMaximumHullPenetration(const FNRawMesh& Hull, const TArray<FVector>& CornerPoints);

	/**
	 * Memoized socket-corner penetration of this junction into its cell's hull, plus the lowest world-Z across
	 * those corners (which anchors the depth readout).
	 *
	 * The ed mode redraws every registered junction on every viewport frame, so this is what stops an idle
	 * viewport from rebuilding the corner points and re-sweeping the hull per junction per frame. Recomputes
	 * only when the hull geometry, this junction's transform, or the socket sizing changes.
	 * @param Hull Cell hull to measure against.
	 * @param SettingsSocketSize Project-wide socket size used to scale the junction's unit socket size.
	 * @param OutMaximumDepth Receives the deepest corner penetration, or 0 when none penetrate.
	 * @param OutLowestCornerZ Receives the lowest world-Z across the socket corners, floored at this junction's
	 *        own component-location Z (matching the readout anchor the draw path has always used).
	 */
	void GetCachedHullPenetration(const FNRawMesh& Hull, const FVector2D& SettingsSocketSize,
		float& OutMaximumDepth, double& OutLowestCornerZ) const;

	/**
	 * Draw the junction's debug visualization through the supplied PDI.
	 * @param bDrawFillDepth Draw the box extruded along the facing axis that previews the volume a filler would
	 *                       occupy. Sized and anchored by the junction's own fill depth and depth mode.
	 */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI,
		const FLinearColor& ValidColor, const FLinearColor& Invalid, bool bShowDepth = false,
		bool bIsConnected = true, bool bDrawBox = true, bool bDrawCornerLines = true, bool bDrawFillDepth = true,
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get()) const;

	/**
	 * @param SocketSize The socket dimensions to size the corners to.
	 * @return The junction's socket corner points in world space.
	 */
	TArray<FVector> GetWorldCornerPoints(const FVector2D& SocketSize) const;

	/**
	 * Spawn a filler actor for this junction when it is left unconnected: select an eligible entry from Fillers
	 * (constraint-gated, then weighted-random), fall back to the project-wide Default Filler when none qualify, and
	 * notify the spawned actor through INCellJunctionFiller.
	 */
	void Fill();

	/**
	 * Re-read this junction's link details from its owning cell, and resolve the junction if it began play before
	 * that cell's assembly data had replicated.
	 *
	 * Called by ANCellLevelInstance::UpdateFromAssemblyData for every junction in the cell. A junction that already
	 * resolved is left alone — its details are refreshed, but nothing is re-driven or unwound.
	 */
	void OnAssemblyDataUpdated();

protected:
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Assembly Operation")
	TWeakObjectPtr<AActor> FillerActor;

	int32 OperationTicket;

	/** Cached level-instance owner when the junction is spawned as part of a streamed-in cell. */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Assembly Operation")
	TWeakObjectPtr<ALevelInstance> LevelInstance;

private:
	/**
	 * Filter this junction's Fillers down to the entries whose context-tag and tag-counter constraints are
	 * satisfied by the generated cell's assembly state, weighted for selection.
	 * @param AssemblyData The generated cell's resolved assembly state (final context tags and tag counter).
	 * @return Indices into Fillers, each inserted Weighting times; empty when every filler is gated out.
	 */
	FNWeightedIntegerArray GetJunctionFillEntries(const FNCellAssemblyData& AssemblyData) const;

	/**
	 * Spawn the resolved project-wide default filler class at this junction and run the shared post-spawn handling.
	 * @param FillerClass The loaded default filler class to spawn.
	 * @param CellLevelInstance The cell level instance that owns this junction.
	 */
	void SpawnDefaultFiller(UClass* FillerClass, ANCellLevelInstance* CellLevelInstance);

	/**
	 * Run the post-spawn handling shared by every filler path: notify the actor through INCellJunctionFiller, register
	 * it with the subsystem under the cell's operation ticket, and cache it as this junction's active filler.
	 * @param SpawnedActor The freshly spawned filler actor.
	 * @param CellLevelInstance The cell level instance that owns this junction.
	 */
	void FinalizeFillerSpawn(AActor* SpawnedActor, ANCellLevelInstance* CellLevelInstance);

	/**
	 * Act on this junction's resolved connection state: drive the additional-actor lists, notify OnBeginPlayTargets,
	 * report a connector endpoint, and fill an unconnected junction according to its requirements.
	 *
	 * Split out of BeginPlay because a junction cannot always answer "am I connected?" at that point — see
	 * BeginPlay for the assembly-data race that defers this to OnAssemblyDataUpdated. Guarded so it runs once.
	 */
	void ResolveConnectionState();

	/**
	 * Drive both additional-actor lists to the visibility this junction's outcome calls for.
	 *
	 * Filled actors are shown only on an unconnected junction, connected actors only on a connected one, and the
	 * connected pass runs second so an actor named in both lists takes its state from AdditionalConnectedActors.
	 * @param bConnected Whether the junction resolved as connected to another junction.
	 * @param bSkipAdditionalFilledActors Hide the filled actors regardless, for a filler entry that opted out of
	 *        the toggle via bSkipAdditionalActors. Never shows them; only ever narrows what bConnected would do.
	 */
	void ProcessAdditionalActors(bool bConnected, bool bSkipAdditionalFilledActors = false);

	/** Guards ResolveConnectionState so BeginPlay and a late OnAssemblyDataUpdated cannot both run it. */
	bool bConnectionStateResolved = false;

	/** Set when BeginPlay found the owning cell's assembly data missing and left the resolve to that cell. */
	bool bAwaitingAssemblyData = false;

	/** One junction's memoized hull penetration plus the inputs it was computed for; reused while those are unchanged. */
	struct FCachedHullPenetration
	{
		/**
		 * CRC over the cell hull's vertex buffer. Used in place of a version counter on the cell root because the
		 * hull is mutated from several places (ANCellActor::CalculateHull / SplitHullEdge, the vertex drag in
		 * FNCellRootComponentVisualizer) and can also be swapped wholesale by undo/redo — a content hash stays
		 * correct without every one of those paths remembering to bump something.
		 */
		uint32 KeyHullVertexCrc = 0;
		/** Hull loop count, so a topology-only change that leaves the vertex buffer identical still invalidates. */
		int32 KeyHullLoopCount = INDEX_NONE;
		FVector KeyLocation = FVector::ZeroVector;
		FRotator KeyRotation = FRotator::ZeroRotator;
		FIntVector2 KeyUnitSocketSize = FIntVector2(0, 0);
		FVector2D KeySettingSocketSize = FVector2D::ZeroVector;

		/** Deepest penetration of any socket corner into the hull. */
		float MaximumDepth = 0.f;
		/** Lowest world-Z across the socket corners; anchors the depth readout beneath the socket. */
		double LowestCornerZ = 0.0;
		/** False until the first computation populates the entry, so a genuine all-zero result still counts as a hit. */
		bool bValid = false;
	};

	/**
	 * Memo backing GetCachedHullPenetration. Lives on the component (rather than in a static map keyed by weak
	 * pointer, as FNBoneComponentVisualizer does) because DrawDebugPDI is the shared entry point for both the
	 * ed-mode render path and FNCellJunctionComponentVisualizer — and because component lifetime then prunes the
	 * entry for free. Mutable so the const draw path can populate it.
	 */
	mutable FCachedHullPenetration CachedHullPenetration;

	N_WORLD_ICON_COMPONENT_HEADER()
};