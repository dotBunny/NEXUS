// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNPrimitiveFontPoint
{
	int8 X;
	int8 Y;
};

/**
 * A simple glyph collection that can be rendered via PDI or DebugDraw calls.
 */
class NEXUSCORE_API FNPrimitiveFont
{
	friend class FNCoreModule;

public:
	constexpr static int UndefinedPointCount = 12;
	static bool IsInitialized() { return bIsInitialized; }
	static void Initialize();
	static TArray<FNPrimitiveFontPoint>& GetGlyph(const char InChar);
	
private:
	static bool bIsInitialized;
	static TArray<TArray<FNPrimitiveFontPoint>> Glyphs;
};