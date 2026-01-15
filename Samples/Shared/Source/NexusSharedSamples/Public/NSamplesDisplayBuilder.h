// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNSamplesDisplayBuilder
{
public:
	static void CreateDisplayInstances(UInstancedStaticMeshComponent* PanelCurve, 
		UInstancedStaticMeshComponent* PanelCurveEdge, UInstancedStaticMeshComponent* PanelMain, 
		UInstancedStaticMeshComponent* PanelCorner, UInstancedStaticMeshComponent* PanelSide,
		FTransform& MainPanelTransform, FTransform& FloorPanelTransform, float Depth, float Width, float Height);
	
	static void CreateScalablePanelInstances(const FTransform& BaseTransform, UInstancedStaticMeshComponent* PanelMain, 
		UInstancedStaticMeshComponent* PanelCorner, UInstancedStaticMeshComponent* PanelSide, float Depth, float Width, 
		bool bIgnoreMainPanel = false);
	
	static void CreateShadowBoxInstances(UInstancedStaticMeshComponent* ShadowBoxSide, 
		UInstancedStaticMeshComponent* ShadowBoxTop, UInstancedStaticMeshComponent* ShadowBoxRound, 
		float ShadowBoxCoverDepthPercentage, float Depth, float Width, float Height);
	
	static void CreateTitlePanelInstances(UInstancedStaticMeshComponent* TitleBarMain, UInstancedStaticMeshComponent* 
		TitleBarEndLeft, UInstancedStaticMeshComponent* TitleBarEndRight, float Depth, float Width);
	
	static void ApplyDefaultInstanceStaticMeshSettings(UInstancedStaticMeshComponent* Instance, USceneComponent* PartRoot);

	
private:	
	static void ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform);
};