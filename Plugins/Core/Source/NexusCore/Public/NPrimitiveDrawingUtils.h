// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM()
enum class ENDrawStringAlignment : uint8
{
	DSA_Left					UMETA(DisplayName = "Left"),
	DSA_Center				UMETA(DisplayName = "Center"),
	DSA_Right				UMETA(DisplayName = "Right"),
};

// PIVOT

class NEXUSCORE_API FNPrimitiveDrawingUtils
{
	friend class FNCoreModule;
public:
	static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position, const FRotator& Rotation,
		FLinearColor ForegroundColor, float Scale = 1, const ENDrawStringAlignment Alignment = ENDrawStringAlignment::DSA_Left,
		float Thickness = 8.f);
private:
	static int CharToGlyphIndex(TCHAR Character);
	static void GenerateGlyphs();

	static TArray<TArray<FVector2D>> Glyphs;
};
