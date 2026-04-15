// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DecalComponent.h"
#include "Components/TextRenderComponent.h"
#include "NSamplesDisplayComponents.generated.h"

// #SONARQUBE-DISABLE: This is a collection of components used, and whilst it has a large number, they are all needed.
UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Samples Display Components")
class UNSamplesDisplayComponents : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()	
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY()
	TObjectPtr<USceneComponent> PartRoot;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> PanelMain;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> PanelCorner;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> PanelSide;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> PanelCurve;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> PanelCurveEdge;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarMain;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarEndLeft;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TitleBarEndRight;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxSide;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxTop;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ShadowBoxRound;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Watermark;
	UPROPERTY()
	TObjectPtr<UDecalComponent> NoticeDecalComponent;
	UPROPERTY()
	TObjectPtr<USpotLightComponent> SpotlightComponent;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> TitleTextComponent;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TitleSpacerComponent;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> DescriptionTextComponent;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> NoticeTextComponent;
	UPROPERTY()
	TObjectPtr<UCameraComponent> ScreenshotCameraComponent;
	
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
// #SONARQUBE-ENABLE