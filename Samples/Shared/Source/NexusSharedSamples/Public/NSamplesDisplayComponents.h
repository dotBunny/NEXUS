// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"

class FNSamplesDisplayComponents
{
public:
	
	TObjectPtr<UInstancedStaticMeshComponent> PanelMain;
	TObjectPtr<UInstancedStaticMeshComponent> PanelCorner;
	TObjectPtr<UInstancedStaticMeshComponent> PanelSide;
	TObjectPtr<UInstancedStaticMeshComponent> PanelCurve;
	TObjectPtr<UInstancedStaticMeshComponent> PanelCurveEdge;
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarMain;
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarEndLeft;
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarEndRight;
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxSide;
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxTop;
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxRound;
	TObjectPtr<UStaticMeshComponent> Watermark;
	
	TObjectPtr<UDecalComponent> NoticeDecalComponent;
	TObjectPtr<USpotLightComponent> SpotlightComponent;
	TObjectPtr<UTextRenderComponent> TitleTextComponent;
	TObjectPtr<UStaticMeshComponent> TitleSpacerComponent;
	TObjectPtr<UTextRenderComponent> DescriptionTextComponent;
	TObjectPtr<UTextRenderComponent> NoticeTextComponent;
	
	void ClearInstances() const
	{
		PanelMain->ClearInstances();
		PanelCorner->ClearInstances();
		PanelSide->ClearInstances();
		PanelCurve->ClearInstances();
		PanelCurveEdge->ClearInstances();

		TitleBarMain->ClearInstances();
		TitleBarEndLeft->ClearInstances();
		TitleBarEndRight->ClearInstances();

		ShadowBoxSide->ClearInstances();
		ShadowBoxTop->ClearInstances();
		ShadowBoxRound->ClearInstances();
	}
	
	void UpdateCollisions(const bool bCollisionEnabled) const
	{
		if (bCollisionEnabled)
		{
			PanelMain->SetCollisionProfileName(FName("BlockAll"));
			PanelCorner->SetCollisionProfileName(FName("BlockAll"));
			PanelSide->SetCollisionProfileName(FName("BlockAll"));
			PanelCurve->SetCollisionProfileName(FName("BlockAll"));
			PanelCurveEdge->SetCollisionProfileName(FName("BlockAll"));
		
			TitleBarMain->SetCollisionProfileName(FName("BlockAll"));
			TitleBarEndLeft->SetCollisionProfileName(FName("BlockAll"));
			TitleBarEndRight->SetCollisionProfileName(FName("BlockAll"));
		
			ShadowBoxSide->SetCollisionProfileName(FName("BlockAll"));
			ShadowBoxTop->SetCollisionProfileName(FName("BlockAll"));
			ShadowBoxRound->SetCollisionProfileName(FName("BlockAll"));
		
			Watermark->SetCollisionProfileName(FName("BlockAll"));
		}
		else
		{
			PanelMain->SetCollisionProfileName(FName("NoCollision"));
			PanelCorner->SetCollisionProfileName(FName("NoCollision"));
			PanelSide->SetCollisionProfileName(FName("NoCollision"));
			PanelCurve->SetCollisionProfileName(FName("NoCollision"));
			PanelCurveEdge->SetCollisionProfileName(FName("NoCollision"));
		
			TitleBarMain->SetCollisionProfileName(FName("NoCollision"));
			TitleBarEndLeft->SetCollisionProfileName(FName("NoCollision"));
			TitleBarEndRight->SetCollisionProfileName(FName("NoCollision"));
		
			ShadowBoxSide->SetCollisionProfileName(FName("NoCollision"));
			ShadowBoxTop->SetCollisionProfileName(FName("NoCollision"));
			ShadowBoxRound->SetCollisionProfileName(FName("NoCollision"));

			Watermark->SetCollisionProfileName(FName("NoCollision"));
		}
	}
};
