// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A single glyph-grid point used when rasterising characters in FNPrimitiveFont.
 *
 * Coordinates are stored in the font's internal grid space (not world units); pairs of points
 * are interpreted as line-segment endpoints by the drawing helpers.
 */
struct FNPrimitiveFontPoint
{
	/** X coordinate of the point within the glyph grid. */
	int8 X;

	/** Y coordinate of the point within the glyph grid. */
	int8 Y;
};

// #SONARQUBE-DISABLE-CPP_S107 Necessary verbosity of methods to fully convey the drawing methods

/**
 * A simple glyph collection that can be rendered via PDI or LineBatchComponent.
 *
 * Provides line-based primitive rendering of ASCII strings for debug and developer visualisation,
 * without relying on UE's Slate or UMG text systems. Glyph data is built once during module
 * startup by FNCoreModule.
 */
class NEXUSCORE_API FNPrimitiveFont
{
	friend class FNCoreModule;

public:
	/** Number of line-segment points used to draw an "undefined" fallback glyph. */
	constexpr static int UndefinedPointCount = 12;

	/**
	 * Indicates whether the glyph table has been populated and the font is safe to use.
	 * @return true once Initialize() has run successfully.
	 */
	static bool IsInitialized() { return bIsInitialized; }

	/** Populates the glyph table; called once by FNCoreModule::StartupModule(). */
	static void Initialize();

	/**
	 * Returns the points that describe the glyph for a given ASCII character.
	 * @param InChar The character to look up.
	 * @return The array of glyph points; an "undefined" glyph is returned for non-printable input.
	 */
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
