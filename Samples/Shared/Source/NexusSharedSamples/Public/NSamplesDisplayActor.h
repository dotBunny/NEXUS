// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "FunctionalTest.h"
#include "NColor.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Engine/Font.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NSamplesDisplayActor.generated.h"

class USpotLightComponent;

/**
 * A display used in NEXUS demonstration levels
 * @remarks Yes, we did rebuild/nativize Epic's content display blueprint!
 */
UCLASS(MinimalAPI, BlueprintType)
class ANSamplesDisplayActor : public AFunctionalTest
{
	GENERATED_BODY()

	explicit ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer);
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

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

	// ~AFunctionalTest Interface
	virtual void PrepareTest() override;
	virtual void CleanUp() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;
	// ~AFunctionalTest Interface

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check True")
	void CheckTrue(const bool bResult, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check False")
	void CheckFalse(const bool bResult, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check True (w/ Location)")
	void CheckTrueWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check False (w/ Location)")
	void CheckFalseWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check True (w/ Object)")
	void CheckTrueWithObject(const bool bResult, const UObject* Object, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check False (w/ Object)")
	void CheckFalseWithObject(const bool bResult, const UObject* Object, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check True (w/ Actor)")
	void CheckTrueWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check False (w/ Actor)")
	void CheckFalseWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check Pass Count ++")
	void IncrementCheckPassCount() { CheckPassCount++; };
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Display|Testing", DisplayName="Check Fail Count ++")
	void IncrementCheckFailCount() { CheckFailCount++; };
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display", DisplayName="Collisions?", meta=(ToolTip="Should the collision profile be setup for the display?"))
	bool bCollisionEnabled = false;

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

	// TIMER
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Enabled")
	bool bTimerEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Duration", meta=(ClampMin=0, ClampMax=30))
	float TimerDuration = 2.f;
	
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Point", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawPoint(UObject* WorldContextObject, const FVector Location, const int TimerIntervals = 1)
	{
		UKismetSystemLibrary::DrawDebugPoint(WorldContextObject, Location, 15.f, FNColor::GetLinearColor(ENColor::NC_Red), TimerDuration * TimerIntervals);
	}
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Sphere", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawSphere(UObject* WorldContextObject, const FVector Location, float Radius, const int TimerIntervals = 1)
	{
		UKismetSystemLibrary::DrawDebugSphere(WorldContextObject, Location, Radius, 24, FNColor::GetLinearColor(ENColor::NC_White), TimerDuration * TimerIntervals, 0.25f);
	}

	UFUNCTION(BlueprintCallable,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Combo Sphere", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawComboSphere(UObject* WorldContextObject, const FVector Location, FVector2D InnerOuter, const int TimerIntervals = 1)
	{
		UKismetSystemLibrary::DrawDebugSphere(WorldContextObject, Location, InnerOuter.X, 24, FNColor::GetLinearColor(ENColor::NC_Black), TimerDuration * TimerIntervals, 0.25f);
		UKismetSystemLibrary::DrawDebugSphere(WorldContextObject, Location, InnerOuter.Y, 24, FNColor::GetLinearColor(ENColor::NC_White), TimerDuration * TimerIntervals, 0.25f);
	}

	UFUNCTION(BlueprintCallable,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Box", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawBox(UObject* WorldContextObject, const FVector Location, const FBox& Dimensions, const int TimerIntervals = 1)
	{
		UKismetSystemLibrary::DrawDebugBox(WorldContextObject, Location, Dimensions.GetExtent(), FNColor::GetLinearColor(NC_White), FRotator::ZeroRotator, TimerDuration * TimerIntervals, 0.25f);
	}

	UFUNCTION(BlueprintImplementableEvent)
	void OnTimerExpired();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Has Test Logic?")
	bool bTestEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Iteration Count")
	int TestIterationCount = 24;
	
private:
	void TimerExpired();
	
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
	void UpdateCollisions() const;
	void UpdateTestComponents();

	void DefaultInstanceStaticMesh(UInstancedStaticMeshComponent* Instance) const;

	float GetTitleOffset() const;
	float GetTitleSpacerOffset() const;
	
	float TextAlignmentOffset(float WidthAdjustment, bool bForceCenter) const;
	FTransform TitlePanelTextTransform() const;
	FTransform TitleTextTransform() const;



	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<USceneComponent> PartRoot;
	
	UPROPERTY()
	TObjectPtr<UDecalComponent> NoticeDecalComponent;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> SpotlightComponent;
	
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> TitleTextComponent;

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

	FTimerHandle TimerHandle;

	int CheckPassCount = 0;
	int CheckFailCount = 0;
};