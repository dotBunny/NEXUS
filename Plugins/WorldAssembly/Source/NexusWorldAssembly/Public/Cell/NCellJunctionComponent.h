// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellAssemblyData.h"
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

/**
 * Scene component marking a junction on a cell — a connection point that lets one cell attach to another during generation.
 *
 * Carries the junction's shape (FNCellJunctionDetails) and provides derived data (corner points, color) used by
 * the graph builder and debug visualizers to reason about cell connectivity.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Cell Junction", meta=(BlueprintSpawnableComponent,
	DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/junction-component"),
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

		N_WORLD_ICON_SCENE_COMPONENT("/NexusWorldAssembly/EditorResources/S_NCellJunctionComponent", this, false, 0.35f)
	}

public:
	/** Junction shape, orientation, and flags authored per junction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cell Junction")
	FNCellJunctionDetails Details;

	/** Candidate fillers for this junction; one is selected (constraint-gated, then weighted-random) and spawned when the junction is left unconnected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cell Junction", meta=(TitleProperty="{Actor}"))
	TArray<FNCellJunctionFillerEntry> Fillers;

	/** When true, bypass filler time-slicing and spawn this junction's filler immediately during BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Spawn Filler Immediately", Category = "Cell Junction",
		meta=(ToolTip="Override timeslicing support and immediately spawn this filler in BeginPlay"))
	bool bSpawnFillerImmediately = false;

	UPROPERTY(EditInstanceOnly, DisplayName="BeginPlay", Category = "Callbacks",
		meta=(AllowedClasses="/Script/NexusWorldAssembly.NCellJunctionBeginPlay"))
	TArray<TObjectPtr<AActor>> OnBeginPlayCallback;

	/** Connection state for this junction, resolved during generation; its bConnected flag gates whether the junction is filled at BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly Operation")
	FNCellLinkDetails LinkDetails;

	//~USceneComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//End USceneComponent

	/** @return The rotational offset authored on the junction's details. */
	FRotator GetOffsetRotator() const;

	/** @return The location offset authored on the junction's details. */
	FVector GetOffsetLocation() const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR

	virtual void PostEditImport() override;

	/** @return The display name of this junction. */
	FString GetJunctionName() const;
	/** Transform-changed callback that keeps the junction's hull-derived data in sync after the component moves. */
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
#endif // WITH_EDITOR

	/** Draw the junction's debug visualization through the supplied PDI. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI,
		const FLinearColor& ValidColor, const FLinearColor& Invalid, bool bShowDepth = false,
		bool bIsConnected = true, bool bDrawBox = true, bool bDrawCornerLines = true,
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

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Assembly Operation")
	TWeakObjectPtr<AActor> FillerActor;

	int32 OperationTicket;

	/** Cached level-instance owner when the junction is spawned as part of a streamed-in cell. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Assembly Operation")
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

	N_WORLD_ICON_HEADER()
};