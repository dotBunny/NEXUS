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
};


USTRUCT(BlueprintType)
struct FNSamplesDisplayTitleSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Title", DisplayName = "Color")
	ENColor TitleColor = ENColor::NC_White;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayDescriptionSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Scale")
	float DescriptionScale = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Paragraph Spacing", meta=(ClampMin=0, ClampMax=10))
	int ParagraphSpacing = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Line Spacing", meta=(ClampMin=0, ClampMax=10))
	int LineSpacing = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Color")
	ENColor DescriptionColor = ENColor::NC_White;
};

USTRUCT(BlueprintType)
struct FNSamplesDisplayScreenshotSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Override Name")
	FText ScreenshotCameraName;
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
};