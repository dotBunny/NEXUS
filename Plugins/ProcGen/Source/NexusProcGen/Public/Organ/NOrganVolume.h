// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameFramework/Volume.h"
#include "NBoneComponent.h"
#include "Organ/NOrganComponent.h"
#include "NOrganVolume.generated.h"

UENUM(BlueprintType)
enum class ENOrganBoneMode : uint8
{
	Automatic = 0,
	Manual = 1,
	Disabled = 2
};

UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "Organ Volume", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSPROCGEN_API ANOrganVolume : public AVolume
{
	
	GENERATED_BODY()

public:
	explicit ANOrganVolume(const FObjectInitializer& ObjectInitializer);
	
	UNOrganComponent* GetOrganComponent() const { return OrganComponent; }

	UNBoneComponent* GetBoneComponent() const { return BoneComponent; }
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "NEXUS")
	ENOrganBoneMode BoneMode = ENOrganBoneMode::Automatic;

protected:	

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NEXUS")
	TObjectPtr<UNOrganComponent> OrganComponent;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NEXUS")
	TObjectPtr<UNBoneComponent> BoneComponent;
};