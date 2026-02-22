// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "NSamplesDisplaySettings.generated.h"

USTRUCT(BlueprintType)
struct FNSamplesDisplaySettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENColor Color = ENColor::NC_Black;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 6.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Depth = 5.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 4.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EHorizTextAligment> TextAlignment = EHTA_Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFloorText = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="ShadowBox Percentage", meta=(ToolTip="What percentage of the depth should the ShadowBox cover? If the Depth <= 1, it will be disabled."))
	float ShadowBoxCoverDepthPercentage = 0.333f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Collisions?", meta=(ToolTip="Should the collision profile be setup for the display?"))
	bool bCollisionEnabled = false;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayTitleSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Color")
	ENColor TitleColor = ENColor::NC_White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Text")
	FText TitleText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Scale")
	float TitleScale = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Text Offset")
	FVector2D TitleTextOffset = FVector2D::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Seperate Panel?")
	bool bSeparateTitlePanel = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Horizontal Rule?")
	bool bShowTitleHorizontalRule = true;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayDescriptionSettings
{
	GENERATED_BODY()
	
	// DESCRIPTION
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Text")
	TArray<FText> DescriptionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Text Padding", meta=(ClampMin=0, ClampMax=100.f))
	float DescriptionTextPadding = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Scale")
	float DescriptionScale = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Paragraph Spacing", meta=(ClampMin=0, ClampMax=10))
	int ParagraphSpacing = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Line Spacing", meta=(ClampMin=0, ClampMax=10))
	int LineSpacing = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Color")
	ENColor DescriptionColor = ENColor::NC_White;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FText CachedDescription;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayScreenshotSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Camera Name Override")
	FText CameraNameOverride;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Filename Override")
	FString FilenameOverride;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayWatermarkSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Enabled")
	bool bWatermarkEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Scale")
	float WatermarkScale = 2.25f;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayNoticeSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Enabled")
	bool bNoticeEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Color")
	ENColor NoticeColor = ENColor::NC_White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Text")
	FText NoticeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Scale")
	float NoticeScale = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Depth", meta=(ClampMin=0, ClampMax=256))
	float NoticeDepth = 128.f;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplaySpotlightSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Enabled")
	bool bSpotlightEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Intensity")
	float SpotlightIntensity = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Attenuation Radius")
	float SpotlightAttenuationRadius = 722.770935f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Inner Cone Angle")
	float SpotlightInnerConeAngle = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Outer Cone Angle")
	float SpotlightOuterConeAngle = 64.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Temperature")
	float SpotlightTemperature = 5500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Cast Shadows")
	bool bSpotlightCastShadows = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Cast Volumetric Shadows")
	bool bSpotlightCastVolumetricShadow = false;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayTimerSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Enabled")
	bool bTimerEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Duration", meta=(ClampMin=0, ClampMax=30))
	float TimerDuration = 2.f;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayTestingSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Iteration Count")
	int TestIterationCount = 24;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Disable Timer")
	bool bTestDisableTimer = false;
};