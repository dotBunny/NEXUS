// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class NEXUSCORE_API FNPrimitiveDrawingUtils
{
	friend class FNCoreModule;
public:
	static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position, const FRotator& Rotation, FLinearColor ForegroundColor, float Scale = 1, float Thickness = 8.f);
private:
	static int CharToGlyphIndex(TCHAR Character);
	static void GenerateGlyphs();

	static TArray<TArray<FVector2D>> Glyphs;
};
