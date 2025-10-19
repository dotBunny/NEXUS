// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPrimitiveDrawStringSettings.generated.h"

UENUM()
enum class ENPrimitiveDrawStringAlignment : uint8
{
	PDSA_Left				UMETA(DisplayName = "Left"),
	PDSA_Center				UMETA(DisplayName = "Center"), // UGH
	PDSA_Right				UMETA(DisplayName = "Right"),
};


UENUM()
enum class ENPrimitiveDrawStringPivot: uint8
{
	PDSP_TopLeft			UMETA(DisplayName = "Top Left"),
	PDSP_BottomLeft			UMETA(DisplayName = "Bottom Left"),
	PDSP_Center				UMETA(DisplayName = "Center"), // UGH
	PDSP_TopRight			UMETA(DisplayName = "Top Right"),
	PDSP_BottomRight		UMETA(DisplayName = "Bottom Right"),
};

USTRUCT(BlueprintType)
struct FNPrimitiveDrawStringSettings
{
	GENERATED_BODY();
	
	UPROPERTY()
	float Scale = 1.f;

	UPROPERTY()
	FLinearColor ForegroundColor = FLinearColor::White;

	UPROPERTY()
	bool bDrawBackground = false;
	
	UPROPERTY()
	FLinearColor BackgroundColor = FLinearColor::White;
	
	UPROPERTY()
	float Thickness = 8.f;

	UPROPERTY()
	float LineHeight = 2.f;

	UPROPERTY()
	ENPrimitiveDrawStringAlignment Alignment = ENPrimitiveDrawStringAlignment::PDSA_Left;

	UPROPERTY()
	ENPrimitiveDrawStringPivot Pivot = ENPrimitiveDrawStringPivot::PDSP_TopLeft;
};