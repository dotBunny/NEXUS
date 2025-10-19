// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreSettings.h"

struct FNPrimitiveDrawStringPoint
{
	int8 X;
	int8 Y;
};

UENUM()
enum class ENPrimitiveDrawStringDirection: uint8
{
	PDSLD_Down			UMETA(DisplayName = "Down"),
	PDSLD_Up			UMETA(DisplayName = "Up"),
};

class NEXUSCORE_API FNPrimitiveDrawingUtils
{
	friend class FNCoreModule;

public:
	
	static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, 
		const FVector& Position, const FRotator& Rotation, FLinearColor ForegroundColor = FLinearColor::White, float Scale = 1, 
		float LineHeight = 2.f, const ENPrimitiveDrawStringDirection LineDirection = ENPrimitiveDrawStringDirection::PDSLD_Down, 
		float Thickness = 8.f);
	
private:
	static bool bHasGeneratedGlyphs;
	static void GenerateGlyphs();
	
	static TArray<TArray<FNPrimitiveDrawStringPoint>> Glyphs;
};