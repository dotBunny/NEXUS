// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

// still not used #00FFFF
namespace NEXUS::WorldAssembly::DefaultColors
{
	static constexpr FLinearColor BoneValid = FLinearColor(0.061246f,1.f,1.f, 1.f); // #46FFFFFF
	static constexpr FLinearColor BoneInvalid = FLinearColor(0.254152f,0.012983f,1.f, 1.f); // #8A1EFFFF

	static constexpr FLinearColor JunctionValid = FLinearColor(0.010330f,1.f,0.391573f, 1.f); // #1AFFA8FF
	static constexpr FLinearColor JunctionInvalid = FLinearColor(1.f,0.097587f,1.f, 1.f); // #FF58FFFF
	static constexpr FLinearColor JunctionUnfilled = FLinearColor(1.f,1.f,1.f, 1.f); // #FFFFFFFF
	static constexpr FLinearColor JunctionConnectorCorners = FLinearColor(0.947307f,0.745404f,0.964686f, 1.f); // #A06600FF

	static constexpr FLinearColor CellHull = FLinearColor(0.f,0.630757f,1.f, 1.f); // #00D0FFFF
	static constexpr FLinearColor CellBounds = FLinearColor(0.434154f,0.006995f,0.001821f, 1.f); // #B01406FF
}

/**
 * Process-wide cache of the World Assembly debug-draw palette, mirrored from UNWorldAssemblyEditorUserSettings.
 *
 * Exists so the draw paths can read a color without walking the settings CDO per primitive. Deliberately global
 * rather than owned by UNWorldAssemblyEdMode: the palette outlives any one activation of the edit mode, and two of
 * its consumers read it with no mode instance to ask. FNCellJunctionComponentVisualizer draws only while the mode is
 * *inactive* (the mode's own Render covers the active case), FNBoneComponentVisualizer draws regardless, and
 * FNWorldAssemblyEditorModule seeds the cache at startup before any mode has been entered.
 *
 * @note Refresh() must be called whenever the source settings change; UNWorldAssemblyEditorUserSettings does this
 *       from its post-edit hooks, and the edit mode repeats it on Enter().
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorColors
{
public:
	/** Re-read the palette from UNWorldAssemblyEditorUserSettings. Safe to call before any world or mode exists. */
	static void Refresh();

	/** @return Cached bounds overlay color. */
	static const FLinearColor& GetCellBounds() { return CellBounds; }

	/** @return Cached hull overlay color. */
	static const FLinearColor& GetCellHull() { return CellHull; }

	/** @return Cached color for a junction that resolved to a valid connection. */
	static const FLinearColor& GetJunctionValid() { return JunctionValid; }

	/** @return Cached color for a junction that failed to resolve. */
	static const FLinearColor& GetJunctionInvalid() { return JunctionInvalid; }

	/** @return Cached color for a junction left open with no filler. */
	static const FLinearColor& GetJunctionUnfilled() { return JunctionUnfilled; }

	/** @return Cached color for the curves bounding a junction connector's route. */
	static const FLinearColor& GetJunctionConnectorCorners() { return JunctionConnectorCorners; }

	/** @return Cached color for a bone that resolved. */
	static const FLinearColor& GetBoneValid() { return BoneValid; }

	/** @return Cached color for a bone that failed to resolve. */
	static const FLinearColor& GetBoneInvalid() { return BoneInvalid; }

private:
	static FLinearColor CellBounds;
	static FLinearColor CellHull;
	static FLinearColor JunctionValid;
	static FLinearColor JunctionInvalid;
	static FLinearColor JunctionUnfilled;
	static FLinearColor JunctionConnectorCorners;
	static FLinearColor BoneValid;
	static FLinearColor BoneInvalid;
};
