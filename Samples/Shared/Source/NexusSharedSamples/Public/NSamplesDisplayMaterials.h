// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSamplesDisplayMaterials.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Samples Display Materials")
class UNSamplesDisplayMaterials : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DisplayMaterial;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> DisplayMaterialInterface;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> NoticeMaterial;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> NoticeMaterialInterface;
	UPROPERTY()
	TObjectPtr<UTextureLightProfile> SpotlightLightProfile;
};