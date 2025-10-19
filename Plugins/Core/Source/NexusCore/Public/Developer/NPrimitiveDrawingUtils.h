// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreSettings.h"

struct FNPrimitiveDrawStringPoint
{
	int8 X;
	int8 Y;
};

class NEXUSCORE_API FNPrimitiveDrawingUtils
{
	friend class FNCoreModule;
public:

	FORCEINLINE static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position, const FRotator& Rotation)
	{
		DrawString(PDI, String, Position, Rotation, UNCoreSettings::Get()->PrimitiveDrawStringSettings);
	}
	FORCEINLINE static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position, const FRotator& Rotation, const FNPrimitiveDrawStringSettings& Settings)
	{
		DrawString(PDI, String, Position, Rotation, Settings.ForegroundColor, Settings.Scale, Settings.Alignment, Settings.Pivot, Settings.LineHeight, Settings.Thickness);
	}

	static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position, const FRotator& Rotation,
		FLinearColor ForegroundColor, float Scale = 1, const ENPrimitiveDrawStringAlignment Alignment = ENPrimitiveDrawStringAlignment::PDSA_Left,
		const ENPrimitiveDrawStringPivot Pivot = ENPrimitiveDrawStringPivot::PDSP_TopLeft, float LineHeight = 2.f, float Thickness = 8.f);
	
private:
	static bool bHasGeneratedGlyphs;
	static void GenerateGlyphs();
	
	static TArray<TArray<FNPrimitiveDrawStringPoint>> Glyphs;
};