// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
// ReSharper disable CppUnusedIncludeDirective
#include "NColor.h"
#include "Engine/Font.h"
#include "Kismet/KismetSystemLibrary.h"
// ReSharper restore CppUnusedIncludeDirective
#include "NSamplesDisplayComponents.h"
#include "NSamplesDisplayMaterials.h"
#include "NSamplesDisplayTest.h"
#include "NSamplesDisplayActor.generated.h"

class UCameraComponent;
class USpotLightComponent;

/**
 * A display used in NEXUS demonstration levels
 * @notes Yes, we did rebuild/nativize Epic's content display blueprint!
 */
UCLASS(DisplayName = "NEXUS: Samples Display Actor", BlueprintType, HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, Actor, Input,
	DataLayers, LevelInstance, WorldPartition, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSSHAREDSAMPLES_API ANSamplesDisplayActor : public AActor
{
	friend class ANSamplesPawn;
	friend class UNSamplesDisplayLibrary;
	friend class ANFunctionalTest;
	
	GENERATED_BODY()

	explicit ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer);
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginDestroy() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	static TArray<ANSamplesDisplayActor*> KnownDisplays;
	
	UFUNCTION(BlueprintCallable)
	void Rebuild();
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Prepare Test"))
	void ReceivePrepareTest();
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Start Test"))
	void ReceiveStartTest();
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Test Finished"))
	void ReceiveTestFinished();
	
	void PrepareTest();
	void StartTest();
	void CleanupTest();
	
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
	ENColor Color = ENColor::NC_Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display", DisplayName="Collisions?", meta=(ToolTip="Should the collision profile be setup for the display?"))
	bool bCollisionEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Text")
	FText TitleText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Scale")
	float TitleScale = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Color")
	ENColor TitleColor = ENColor::NC_White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Seperate Panel")
	bool bSeparateTitlePanel = false;
	
	// DESCRIPTION
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text")
	TArray<FText> DescriptionText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Scale")
	float DescriptionScale = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Line Spacing", meta=(ClampMin=0, ClampMax=10))
	int LineSpacing = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Paragraph Spacing", meta=(ClampMin=0, ClampMax=10))
	int ParagraphSpacing = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text Padding", meta=(ClampMin=0, ClampMax=100.f))
	float DescriptionTextPadding = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Color")
	ENColor DescriptionColor = ENColor::NC_White;

	// WATERMARK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Watermark", DisplayName = "Enabled")
	bool bWatermarkEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Watermark", DisplayName = "Scale")
	float WatermarkScale = 2.25f;
	
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
	ENColor NoticeColor = ENColor::NC_White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Text")
	FText NoticeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Scale")
	float NoticeScale = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Notice", DisplayName = "Depth", meta=(ClampMin=0, ClampMax=256))
	float NoticeDepth = 128.f;

	// TIMER
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Enabled")
	bool bTimerEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Duration", meta=(ClampMin=0, ClampMax=30))
	float TimerDuration = 2.f;

	UFUNCTION(BlueprintImplementableEvent)
	void OnTimerExpired();

	// TESTING
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Iteration Count")
	int TestIterationCount = 24;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Disable Timer")
	bool bTestDisableTimer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NEXUS|Cache", DisplayName = "Description")
	FText CachedDescription;
	
	// SCREENSHOT
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NEXUS|Screenshot", DisplayName = "Camera")
	TObjectPtr<UCameraComponent> ScreenshotCameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Screenshot", DisplayName = "Override Name")
	FText ScreenshotCameraName;
	
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Testing")
	TObjectPtr<UNSamplesDisplayTest> TestInstance;
	
private:
	void BuildDescription();

	void TimerExpired();
	float TextAlignmentOffset(float WidthAdjustment, bool bForceCenter) const;
	FTransform TitleTextTransform() const;

	void UpdateDescription() const;
	void UpdateDisplayColor();
	void UpdateNotice();
	void UpdateSpotlight() const;
	void UpdateTitleText() const;
	void UpdateWatermark() const;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<USceneComponent> PartRoot;
	
	UPROPERTY()
	TObjectPtr<UTextureLightProfile> SpotlightLightProfile;

	FTimerHandle TimerHandle;
	
	TUniquePtr<FNSamplesDisplayComponents> Parts;
	TUniquePtr<FNSamplesDisplayMaterials> Materials;

	FTransform MainPanelTransform;
	FTransform FloorPanelTransform;
};