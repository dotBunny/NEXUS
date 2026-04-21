// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameFramework/Volume.h"
#include "NBoneComponent.h"
#include "Organ/NOrganComponent.h"
#include "NOrganVolume.generated.h"

/**
 * Convenience volume actor that bundles an UNOrganComponent and UNBoneComponent together.
 *
 * Dropping one of these into a level is the fastest way to declare a region that ProcGen
 * should populate — the organ component supplies tissue/cell rules and the bone component
 * supplies the spatial skeleton used during graph construction.
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | Organ Volume", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSPROCGEN_API ANOrganVolume : public AVolume
{

	GENERATED_BODY()

public:
	explicit ANOrganVolume(const FObjectInitializer& ObjectInitializer);

	/** @return The bundled organ component. */
	UNOrganComponent* GetOrganComponent() const { return OrganComponent; }

	/** @return The bundled bone component used for the organ's spatial skeleton. */
	UNBoneComponent* GetBoneComponent() const { return BoneComponent; }

protected:

	/** Organ rules (tissues, counts, trigger) for this volume. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NEXUS")
	TObjectPtr<UNOrganComponent> OrganComponent;

	/** Spatial skeleton that shapes cell placement inside the volume. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NEXUS")
	TObjectPtr<UNBoneComponent> BoneComponent;
};