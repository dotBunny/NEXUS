// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorColors.h"

#include "NWorldAssemblyEditorUserSettings.h"

FLinearColor FNWorldAssemblyEditorColors::CellBounds = NEXUS::WorldAssembly::DefaultColors::CellBounds;
FLinearColor FNWorldAssemblyEditorColors::CellHull = NEXUS::WorldAssembly::DefaultColors::CellHull;
FLinearColor FNWorldAssemblyEditorColors::JunctionValid = NEXUS::WorldAssembly::DefaultColors::JunctionValid;
FLinearColor FNWorldAssemblyEditorColors::JunctionInvalid = NEXUS::WorldAssembly::DefaultColors::JunctionInvalid;
FLinearColor FNWorldAssemblyEditorColors::JunctionUnfilled = NEXUS::WorldAssembly::DefaultColors::JunctionUnfilled;
FLinearColor FNWorldAssemblyEditorColors::JunctionConnectorCorners = NEXUS::WorldAssembly::DefaultColors::JunctionConnectorCorners;
FLinearColor FNWorldAssemblyEditorColors::BoneValid = NEXUS::WorldAssembly::DefaultColors::BoneValid;
FLinearColor FNWorldAssemblyEditorColors::BoneInvalid = NEXUS::WorldAssembly::DefaultColors::BoneInvalid;

void FNWorldAssemblyEditorColors::Refresh()
{
	const UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::Get();

	CellHull = Settings->ColorPaletteCellHull;
	CellBounds = Settings->ColorPaletteCellBounds;

	JunctionUnfilled = Settings->ColorPaletteJunctionsUnfilled;
	JunctionValid = Settings->ColorPaletteJunctionsValid;
	JunctionInvalid = Settings->ColorPaletteJunctionsInvalid;
	JunctionConnectorCorners = Settings->ColorPaletteJunctionsConnectorCorners;

	// The bone entries were listed in the settings' post-edit hook but never copied by the cache this replaces, so
	// editing them refreshed nothing and FNBoneComponentVisualizer stayed on the compiled-in defaults for the life of
	// the editor. The mode's own Render reads the settings directly, so the two disagreed once a bone color was changed.
	BoneValid = Settings->ColorPaletteBonesValid;
	BoneInvalid = Settings->ColorPaletteBonesInvalid;
}
