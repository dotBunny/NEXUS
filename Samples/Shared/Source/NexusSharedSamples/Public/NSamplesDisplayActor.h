// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "NColor.h"
#include "Engine/Font.h"
#include "NSamplesDisplayActor.generated.h"

class USpotLightComponent;

/**
 * A display actor used in NEXUS demonstration levels
 * @remarks Yes, we did rebuild/nativize Epic's content display blueprint!
 */
UCLASS(Config = Game)
class NEXUSSHAREDSAMPLES_API ANSamplesDisplayActor : public AActor
{
	GENERATED_BODY()

	explicit ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer);
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	
	UFUNCTION(BlueprintCallable)
	void Rebuild();

	UFUNCTION(BlueprintCallable)
	UActorComponent* GetComponentInstance(TSubclassOf<UActorComponent> ComponentClass);

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	FString GetTaggedMessage(const FString& Message) const { return FString::Printf(TEXT("[%s]: %s"), *GetTitle(), *Message);};

	UFUNCTION(BlueprintCallable)
	FString GetTitle() const { return TitleText.ToString();};

	UFUNCTION(BlueprintCallable)
	FString GetTaggedPrefix() const { return GetTitle(); };

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	float Width = 6.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	float Depth = 5.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	float Height = 4.0;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	TEnumAsByte<EHorizTextAligment> TextAlignment = EHTA_Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	bool bFloorText = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display", DisplayName="ShadowBox Percentage", meta=(ToolTip="What percentage of the depth should the ShadowBox cover? If the Depth <= 1, it will be disabled."))
	float ShadowBoxCoverDepthPercentage = 0.333f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display")
	TEnumAsByte<ENColor> Color = NC_Black;

	// TITLE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Text")
	FText TitleText = FText::FromString("Title");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Scale")
	float TitleScale = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Color")
	TEnumAsByte<ENColor> TitleColor = NC_White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Seperate Panel")
	bool bSeparateTitlePanel = false;

	// DESCRIPTION
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text")
	TArray<FText> DescriptionText = {
		FText::FromString("Example of description, paragraph #1."),
		FText::FromString("Example of description, paragraph #2.")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Scale")
	float DescriptionScale = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Line Spacing", meta=(ClampMin=0, ClampMax=10))
	int LineSpacing = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Paragraph Spacing", meta=(ClampMin=0, ClampMax=10))
	int ParagraphSpacing = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text Padding", meta=(ClampMin=0, ClampMax=100.f))
	float DescriptionTextPadding = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Color")
	TEnumAsByte<ENColor> DescriptionColor = NC_White;

	// SPOTLIGHT
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Enabled")
	bool bSpotlightEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Intensity")
	float SpotlightIntensity = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Attenuation Radius")
	float SpotlightAttenuationRadius = 722.770935f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Inner Cone Angle")
	float SpotlightInnerConeAngle = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Outer Cone Angle")
	float SpotlightOuterConeAngle = 64.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Temperature")
	float SpotlightTemperature = 5500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Cast Shadows")
	bool bSpotlightCastShadows = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Spotlight", DisplayName = "Cast Volumetric Shadows")
	bool bSpotlightCastVolumetricShadow = false;

	// NOTICE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Enabled")
	bool bNoticeEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Color")
	TEnumAsByte<ENColor> NoticeColor = NC_White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Text")
	FText NoticeText = FText::FromString("DEPRECATED");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Scale")
	float NoticeScale = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Depth", meta=(ClampMin=0, ClampMax=256))
	float NoticeDepth = 128.f;
	
private:
	static void ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform);
	
	void CreateDisplayInstances();
	void CreateScalablePanelInstances(const FTransform& BaseTransform, float Length, bool bIgnoreMainPanel = false) const;
	void CreateShadowBoxInstances() const;
	void CreateTitlePanelInstances() const;
	
	void UpdateDescription();
	void UpdateDisplayColor();
	void UpdateNotice();
	void UpdateSpotlight() const;
	void UpdateTitleText() const;

	void DefaultInstanceStaticMesh(UInstancedStaticMeshComponent* Instance) const;

	float GetTitleOffset() const;
	float GetTitleSpacerOffset() const;
	
	float TextAlignmentOffset(float WidthAdjustment, bool bForceCenter) const;
	FTransform TitlePanelTextTransform() const;
	FTransform TitleTextTransform() const;

	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY()
	TObjectPtr<UDecalComponent> NoticeDecalComponent;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> SpotlightComponent;
	
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> TitleTextComponent;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_Main;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_Corner;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_Side;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_Curve;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_CurveEdge;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_TitleBarMain;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_TitleBarEnd;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_TitleBarEndR;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_ShadowStraight;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_ShadowStraightTop;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ISM_ShadowRound;
	

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TitleSpacerComponent;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> DescriptionTextComponent;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> NoticeTextComponent;

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