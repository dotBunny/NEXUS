// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "NCellLinkDetails.h"
#include "NColor.h"
#include "NWorldAssemblySettings.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "NCellJunctionComponent.generated.h"

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

	/**
	 * Compute the four corner points of the junction in component space.
	 * @param SocketUnitSize Project-configured socket unit size used to size the junction.
	 */
	TArray<FVector> GetCornerPoints(const FVector2D& SocketUnitSize) const;

#if WITH_EDITOR

	virtual void PostEditImport() override;
	
	/** @return The display name of this junction. */
	FString GetJunctionName() const;
	/** Transform-changed callback that keeps the junction's hull-derived data in sync after the component moves. */
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
#endif // WITH_EDITOR
	
	/** Draw the junction's debug visualization through the supplied PDI. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, 
		bool bShowDepth = false, FLinearColor DefaultColor = FNColor::GreenLight, 
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get()) const;

	/**
	 * @param SocketSize The socket dimensions to size the corners to.
	 * @return The junction's socket corner points in world space.
	 */
	TArray<FVector> GetWorldCornerPoints(const FVector2D& SocketSize) const;
	
private:
	/** Cached level-instance owner when the junction is spawned as part of a streamed-in cell. */
	TWeakObjectPtr<ALevelInstance> LevelInstance;
	N_WORLD_ICON_HEADER()
};