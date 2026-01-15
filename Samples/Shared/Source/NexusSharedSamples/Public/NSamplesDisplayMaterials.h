// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNSamplesDisplayMaterials
{
public:
	TObjectPtr<UMaterialInstanceDynamic> DisplayMaterial;
	TObjectPtr<UMaterialInterface> DisplayMaterialInterface;
	TObjectPtr<UMaterialInstanceDynamic> NoticeMaterial;
	TObjectPtr<UMaterialInterface> NoticeMaterialInterface;
};
