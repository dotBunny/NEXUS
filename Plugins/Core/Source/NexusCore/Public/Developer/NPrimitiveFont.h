// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNPrimitiveFontPoint
{
	int8 X;
	int8 Y;
};

// #SONARQUBE-DISABLE-CPP_S107 Necessary verbosity of methods to fully convey the drawing methods

/**
 * A simple glyph collection that can be rendered via PDI or LineBatchComponent
 */
class NEXUSCORE_API FNPrimitiveFont
{
	friend class FNCoreModule;

public:
	constexpr static int UndefinedPointCount = 12;
	static bool IsInitialized() { return bIsInitialized; }
	static void Initialize();
	
	FORCEINLINE static TArray<FNPrimitiveFontPoint>& GetGlyph(const char InChar)
	{
		if (InChar < 32 || InChar > 126) return Glyphs[0];
		return Glyphs[InChar];
	}
	
	/**
	 * Draw a string via a PDI.
	 * @param PDI The interface to use for drawing the string.
	 * @param String The string to draw out.
	 * @param Position The world position to start drawing the string at.
	 * @param Rotation The world rotation to apply to the drawing, the base orientation is backwards facing. 
	 * @param ForegroundColor The color to use when drawing the lines for the string.
	 * @param Scale The multiplier to apply to glyph size.
	 * @param LineHeight The height used to represent a line.
	 * @param Thickness The thickness of the lines used to draw glyphs.
	 * @param bInvertLineFeed Should new lines be stacked on top of older lines?
	 * @param bDrawBelowPosition Should the top of the first line align with the position?
	 * @param DepthPriorityGroup What depth should the string be drawn at?
	 */
	static void DrawPDI(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position,
						   const FRotator& Rotation, FLinearColor ForegroundColor = FLinearColor::White, float Scale = 1, 
						   float LineHeight = 4.f,  float Thickness = 8.f, const bool bInvertLineFeed = false,
						   const bool bDrawBelowPosition = true, const ESceneDepthPriorityGroup DepthPriorityGroup = SDPG_World);
	
	/**
	 * Draw a string via a LineBatchComponent.
	 * @param LineBatch The LineBatchComponent to use for drawing the string.
	 * @param String The string to draw out.
	 * @param Position The world position to start drawing the string at.
	 * @param Rotation The world rotation to apply to the drawing, the base orientation is backwards facing. 
	 * @param ForegroundColor The color to use when drawing the lines for the string.
	 * @param Scale The multiplier to apply to glyph size.
	 * @param LineHeight The height used to represent a line.
	 * @param Thickness The thickness of the lines used to draw glyphs.
	 * @param LifeTime The lifetime of the string in seconds, -1 for infinite.
	 * @param bInvertLineFeed Should new lines be stacked on top of older lines?
	 * @param bDrawBelowPosition Should the top of the first line align with the position?
	 * @param DepthPriorityGroup What depth should the string be drawn at?
	 */
	static void DrawBatchString(ULineBatchComponent* LineBatch, FString& String, const FVector& Position,
						   const FRotator& Rotation, FLinearColor ForegroundColor = FLinearColor::White, float Scale = 1, 
						   float LineHeight = 4.f,  float Thickness = 8.f, float LifeTime = 0.f, const bool bInvertLineFeed = false,
						   const bool bDrawBelowPosition = true, const ESceneDepthPriorityGroup DepthPriorityGroup = SDPG_World);
private:
	static bool bIsInitialized;
	static TArray<TArray<FNPrimitiveFontPoint>> Glyphs;
};

// #SONARQUBE-ENABLE
