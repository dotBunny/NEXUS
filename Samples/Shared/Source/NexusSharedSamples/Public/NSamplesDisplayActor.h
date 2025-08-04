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
#include "Components/SplineComponent.h"
#include "Macros/NWorldMacros.h"
#include "NSamplesDisplayActor.generated.h"

class USpotLightComponent;

#define N_TIMER_DRAW_THICKNESS 0.35f

UENUM(BlueprintType)
enum class ESampleTestResult : uint8
{
	Default,
	Invalid,
	Error,
	Running,
	Failed,
	Succeeded
};

/**
 * A display used in NEXUS demonstration levels
 * @remarks Yes, we did rebuild/nativize Epic's content display blueprint!
 */
UCLASS(BlueprintType, HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, Actor, Input,
	DataLayers, LevelInstance, WorldPartition, HLOD, LOD, Rendering, Collision, Physics, Networking, Replication))
class NEXUSSHAREDSAMPLES_API ANSamplesDisplayActor : public AActor
{
	DECLARE_DELEGATE_OneParam(FOnTestEventWithMessageSignature, const FString&);
	DECLARE_DELEGATE_TwoParams(FOnTestFinishEventSignature, ESampleTestResult TestResult, const FString& Message)
	
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




	// CALLBACKS FROM FUNCTIONAL TEST

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddWarning(const FString& Message) { OnTestWarning.ExecuteIfBound(Message); };
	FOnTestEventWithMessageSignature OnTestWarning;
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddError(const FString& Message) { OnTestError.ExecuteIfBound(Message); };
	FOnTestEventWithMessageSignature OnTestError;
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddInfo(const FString& Message) { OnTestInfo.ExecuteIfBound(Message); };
	FOnTestEventWithMessageSignature OnTestInfo;
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void FinishTest(ESampleTestResult TestResult, const FString& Message);
	FOnTestFinishEventSignature OnTestFinish;

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True")
	void CheckTrue(const bool bResult, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False")
	void CheckFalse(const bool bResult, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Count)")
	void CheckTrueWithCount(const bool bResult, const int& Count, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Count)")
	void CheckFalseWithCount(const bool bResult, const int& Count, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Location)")
	void CheckTrueWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Location)")
	void CheckFalseWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Object)")
	void CheckTrueWithObject(const bool bResult, const UObject* Object, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Object)")
	void CheckFalseWithObject(const bool bResult, const UObject* Object, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Actor)")
	void CheckTrueWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Actor)")
	void CheckFalseWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check Pass Count ++")
	void IncrementCheckPassCount() { CheckPassCount++; };
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check Fail Count ++")
	void IncrementCheckFailCount() { CheckFailCount++; };
	
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
	TEnumAsByte<ENColor> Color = NC_Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Display", DisplayName="Collisions?", meta=(ToolTip="Should the collision profile be setup for the display?"))
	bool bCollisionEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Text")
	FText TitleText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Scale")
	float TitleScale = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Color")
	TEnumAsByte<ENColor> TitleColor = NC_White;

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
	TEnumAsByte<ENColor> DescriptionColor = NC_White;

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
	TEnumAsByte<ENColor> NoticeColor = NC_White;

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
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Point", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawPoint(UObject* WorldContextObject, const FVector Location, const int TimerIntervals = 1) const
	{
		DrawDebugPoint(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, 15.f, FNColor::GetColor(NC_Red),
			false, TimerDuration * TimerIntervals, SDPG_World);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Sphere", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawSphere(UObject* WorldContextObject, const FVector Location, const float Radius, const int TimerIntervals = 1) const
	{
		DrawDebugSphere(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, Radius,24,
			FNColor::GetColor(NC_White), false, TimerDuration * TimerIntervals, SDPG_World);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Combo Sphere", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawComboSphere(UObject* WorldContextObject, const FVector Location, const FVector2D InnerOuter, const int TimerIntervals = 1) const
	{
		DrawDebugSphere(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, InnerOuter.X, 24,
			FNColor::GetColor(NC_Black), false,TimerDuration * TimerIntervals, SDPG_World, N_TIMER_DRAW_THICKNESS);
		DrawDebugSphere(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, InnerOuter.Y, 24,
			FNColor::GetColor(NC_White), false, TimerDuration * TimerIntervals, SDPG_World, N_TIMER_DRAW_THICKNESS);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Box", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawBox(UObject* WorldContextObject, const FVector Location, const FBox& Dimensions, const int TimerIntervals = 1) const
	{
		DrawDebugBox(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, Dimensions.GetExtent(),
			FNColor::GetColor(NC_White), false, TimerDuration * TimerIntervals,
			SDPG_World, N_TIMER_DRAW_THICKNESS);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Combo Box", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawComboBox(UObject* WorldContextObject, const FVector Location, const FBox& InnerDimensions, const FBox& OuterDimensions, const int TimerIntervals = 1) const
	{
		DrawDebugBox(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, InnerDimensions.GetExtent(),
			FNColor::GetColor(NC_Black), false, TimerDuration * TimerIntervals,
			SDPG_World, N_TIMER_DRAW_THICKNESS);

		DrawDebugBox(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Location, OuterDimensions.GetExtent(),
		FNColor::GetColor(NC_White), false, TimerDuration * TimerIntervals,
	SDPG_World, N_TIMER_DRAW_THICKNESS);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Circle", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawCircle(UObject* WorldContextObject, const FVector Location, const float& Radius, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1) const
	{
		FMatrix DrawMatrix = BaseDrawMatrix * FRotationMatrix(Rotation);
		DrawMatrix.SetOrigin(Location);
		
		DrawDebugCircle(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), DrawMatrix, Radius, 24,
			FNColor::GetColor(NC_White), false,TimerDuration * TimerIntervals, SDPG_World,
			N_TIMER_DRAW_THICKNESS,false);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Combo Circle", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawComboCircle(UObject* WorldContextObject, const FVector Location, const FVector2D& InnerOuter, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1) const
	{
		FMatrix DrawMatrix = BaseDrawMatrix * FRotationMatrix(Rotation);
		DrawMatrix.SetOrigin(Location);

		DrawDebugCircle(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), DrawMatrix, InnerOuter.X, 24,
			FNColor::GetColor(NC_Black), false, TimerDuration * TimerIntervals,
			SDPG_World, N_TIMER_DRAW_THICKNESS, false);

		DrawDebugCircle(N_GET_WORLD_FROM_CONTEXT(WorldContextObject), DrawMatrix, InnerOuter.Y, 24,
			FNColor::GetColor(NC_White), false,TimerDuration * TimerIntervals, SDPG_World,
			N_TIMER_DRAW_THICKNESS, false);
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Spline", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawSpline(UObject* WorldContextObject, const USplineComponent* Spline, const int TimerIntervals = 1) const
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
		const float LifeTime = TimerDuration * TimerIntervals;
		TArray<FVector> SplinePoints;
		const float SplineLength = Spline->GetSplineLength();
		const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); // One point every ~20 units
		const float DistancePerSegment = SplineLength / NumSegments;
		SplinePoints.Reserve(NumSegments);
		for (int32 i = 0; i <= NumSegments; ++i)
		{
			const float Distance = DistancePerSegment * i;
			const FVector Point = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			SplinePoints.Add(Point);
		}
		for (int32 i = 0; i < SplinePoints.Num() - 1; ++i)
		{
			DrawDebugLine(World, SplinePoints[i], SplinePoints[i + 1], FNColor::GetColor(NC_White),
				false, LifeTime, SDPG_World, N_TIMER_DRAW_THICKNESS);
		}
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Rectangle", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawRectangle(UObject* WorldContextObject, const FVector Location, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, int TimerIntervals = 1) const
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
		const float LifeTime = TimerDuration * TimerIntervals;
		TArray<FVector> RectanglePoints;
		RectanglePoints.Reserve(5);
		
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			RectanglePoints.Add(Location + FVector(-ExtentX, -ExtentY, 0));
			RectanglePoints.Add(Location + FVector(-ExtentX, ExtentY, 0));
			RectanglePoints.Add(Location + FVector(ExtentX, ExtentY, 0));
			RectanglePoints.Add(Location + FVector(ExtentX, -ExtentY, 0));
		
			RectanglePoints.Add(Location + FVector(-ExtentX, -ExtentY, 0));
		}
		else
		{
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, -ExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, ExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(ExtentX, ExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(ExtentX, -ExtentY, 0)));
		
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, -ExtentY, 0)));
		}
		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(NC_White),
				false, LifeTime, SDPG_World, N_TIMER_DRAW_THICKNESS);
		}
	}
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Display|Timer", DisplayName="Timer: Draw Combo Rectangle", meta = (WorldContext = "WorldContextObject"))
	void TimerDrawComboRectangle(UObject* WorldContextObject, const FVector Location, const FVector2D& InnerDimensions, const FVector2D& OuterDimensions, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1) const
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
		const float LifeTime = TimerDuration * TimerIntervals;
		TArray<FVector> RectanglePoints;
		RectanglePoints.Reserve(5);
		
		const float InnerExtentX = InnerDimensions.X * 0.5f;
		const float InnerExtentY = InnerDimensions.Y * 0.5f;
		const float OuterExtentX = OuterDimensions.X * 0.5f;
		const float OuterExtentY = OuterDimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			RectanglePoints.Add(Location + FVector(-InnerExtentX, -InnerExtentY, 0));
			RectanglePoints.Add(Location + FVector(-InnerExtentX, InnerExtentY, 0));
			RectanglePoints.Add(Location + FVector(InnerExtentX, InnerExtentY, 0));
			RectanglePoints.Add(Location + FVector(InnerExtentX, -InnerExtentY, 0));

			RectanglePoints.Add(Location + FVector(-InnerExtentX, -InnerExtentY, 0));
		}
		else
		{
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, -InnerExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, InnerExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(InnerExtentX, InnerExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(InnerExtentX, -InnerExtentY, 0)));

			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, -InnerExtentY, 0)));
		}
		
		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(NC_Black),
				false, LifeTime, SDPG_World, N_TIMER_DRAW_THICKNESS);
		}

		RectanglePoints.Empty();

		if (Rotation.IsZero())
		{
			RectanglePoints.Add(Location + FVector(-OuterExtentX, -OuterExtentY, 0));
			RectanglePoints.Add(Location + FVector(-OuterExtentX, OuterExtentY, 0));
			RectanglePoints.Add(Location + FVector(OuterExtentX, OuterExtentY, 0));
			RectanglePoints.Add(Location + FVector(OuterExtentX, -OuterExtentY, 0));
		
			RectanglePoints.Add(Location + FVector(-OuterExtentX, -OuterExtentY, 0));
		}
		else
		{
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, -OuterExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, OuterExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(OuterExtentX, OuterExtentY, 0)));
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(OuterExtentX, -OuterExtentY, 0)));
		
			RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, -OuterExtentY, 0)));
		}
		
		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(NC_White),
				false, LifeTime, SDPG_World, N_TIMER_DRAW_THICKNESS);
		}
	}

	UFUNCTION(BlueprintImplementableEvent)
	void OnTimerExpired();

	// TESTING
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Iteration Count")
	int TestIterationCount = 24;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Disable Timer")
	bool bTestDisableTimer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NEXUS|Cache", DisplayName = "Description")
	FText CachedDescription;
	
private:
	static void ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform);

	void TimerExpired();
	
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
	void UpdateWatermark() const;

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
	TObjectPtr<UStaticMeshComponent> Watermark;
	

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


	FTransform MainPanelTransform;
	FTransform FloorPanelTransform;
	FMatrix BaseDrawMatrix = FRotationMatrix::MakeFromYZ(FVector::ForwardVector, FVector::LeftVector);
	
	int CheckPassCount = 0;
	int CheckFailCount = 0;
};