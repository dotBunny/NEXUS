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
		
	UFUNCTION(BlueprintPure, Category = "NEXUS|Samples Display|Test Settings")
	int GetTestIterationCount() const { return TestingSettings.TestIterationCount; }
	
	void PrepareTest();
	void StartTest();
	void CleanupTest();
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Base")
	FNSamplesDisplaySettings BaseSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Title")
	FNSamplesDisplayTitleSettings TitleSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Description")
	FNSamplesDisplayDescriptionSettings DescriptionSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Watermark")
	FNSamplesDisplayWatermarkSettings WatermarkSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Spotlight")
	FNSamplesDisplaySpotlightSettings SpotlightSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Notice")
	FNSamplesDisplayNoticeSettings NoticeSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Screenshot")
	FNSamplesDisplayScreenshotSettings ScreenshotSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Timer")
	FNSamplesDisplayTimerSettings TimerSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples Display", DisplayName = "Testing")
	FNSamplesDisplayTestingSettings TestingSettings;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNSamplesDisplayTest> TestInstance;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnTimerExpired();
	
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