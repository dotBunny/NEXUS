// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNSamplesDisplayBuilder
{
public:
	static void CreateScalablePanelInstances(const FTransform& BaseTransform, UInstancedStaticMeshComponent* PanelMain, UInstancedStaticMeshComponent* PanelCorner, UInstancedStaticMeshComponent* PanelSide, float Length, float Width, bool bIgnoreMainPanel = false);
private:	
	static void ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform);
};