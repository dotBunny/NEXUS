// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoneComponent.generated.h"

UCLASS(ClassGroup=(Nexus), meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNBoneComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
};
	