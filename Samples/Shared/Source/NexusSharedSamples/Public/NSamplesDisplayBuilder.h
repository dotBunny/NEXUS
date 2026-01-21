// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSamplesDisplayComponents.h"

class FNSamplesDisplayBuilder
{
public:
	static void CreateDisplayInstances(FNSamplesDisplayComponents* Components,
		FTransform& MainPanelTransform, FTransform& FloorPanelTransform, float Depth, float Width, float Height);
	
	static void CreateScalablePanelInstances(const FTransform& BaseTransform, const FNSamplesDisplayComponents* Components, 
		float Depth, float Width, bool bIgnoreMainPanel = false);
	
	static void CreateShadowBoxInstances(const FNSamplesDisplayComponents* Components, 
		float ShadowBoxCoverDepthPercentage, float Depth, float Width, float Height);
	
	static void CreateTitlePanelInstances(const FNSamplesDisplayComponents* Components, float Depth, float Width);
	
	static void ApplyDefaultInstanceStaticMeshSettings(UInstancedStaticMeshComponent* Instance, USceneComponent* PartRoot);
	
private:	
	static void ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform);
};
