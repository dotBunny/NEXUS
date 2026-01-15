// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
// ReSharper disable CppUnusedIncludeDirective
#include "NColor.h"
#include "Engine/Font.h"
#include "Kismet/KismetSystemLibrary.h"
// ReSharper restore CppUnusedIncludeDirective
#include "NSamplesDisplayComponents.h"
#include "NSamplesDisplayMaterials.h"
#include "NSamplesDisplaySettings.h"
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
	ENColor Color = ENColor::NC_Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display", DisplayName="Collisions?", meta=(ToolTip="Should the collision profile be setup for the display?"))
	bool bCollisionEnabled = false;

	// TITLE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Text")
	FText TitleText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Scale")
	float TitleScale = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Seperate Panel")
	bool bSeparateTitlePanel = false;
	
	// DESCRIPTION
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text")
	TArray<FText> DescriptionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Description", DisplayName = "Text Padding", meta=(ClampMin=0, ClampMax=100.f))
	float DescriptionTextPadding = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NEXUS|Description", DisplayName = "Cached Description")
	FText CachedDescription;
	
	// TIMER
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Enabled")
	bool bTimerEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Timer", DisplayName = "Duration", meta=(ClampMin=0, ClampMax=30))
	float TimerDuration = 2.f;

	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Testing")
	TObjectPtr<UNSamplesDisplayTest> TestInstance;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTimerExpired();

	// TESTING
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Iteration Count")
	int TestIterationCount = 24;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Disable Timer")
	bool bTestDisableTimer = false;



	// SETTINGS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Base")
	FNSamplesDisplaySettings BaseSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Title")
	FNSamplesDisplayTitleSettings TitleSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Description")
	FNSamplesDisplayDescriptionSettings DescriptionSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Watermark")
	FNSamplesDisplayWatermarkSettings WatermarkSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Spotlight")
	FNSamplesDisplaySpotlightSettings SpotlightSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Notice")
	FNSamplesDisplayNoticeSettings NoticeSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings", DisplayName = "Screenshot")
	FNSamplesDisplayScreenshotSettings ScreenshotSettings;
	
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

	FTimerHandle TimerHandle;
	
	TUniquePtr<FNSamplesDisplayComponents> Parts;
	TUniquePtr<FNSamplesDisplayMaterials> Materials;

	FTransform MainPanelTransform;
	FTransform FloorPanelTransform;
};