// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"

/**
 * The category commands driving the World Assembly edit mode's toolkit rail.
 *
 * One command per category button down the left edge of the mode panel; activating one swaps the palette shown
 * beside it. Kept separate from the per-category command classes that hold the actions the palettes are filled with —
 * FToolkitBuilder wants the category and its contents supplied as two distinct lists.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class FNWorldAssemblyEdModePaletteCommands final : public TCommands<FNWorldAssemblyEdModePaletteCommands>
{
public:
	FNWorldAssemblyEdModePaletteCommands()
		: TCommands<FNWorldAssemblyEdModePaletteCommands>(
			TEXT("NWorldAssemblyEdModePaletteCommands"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NWorldAssemblyEdModePaletteCommands", "World Assembly Editor Mode Palettes"),
			NAME_None,
			FNWorldAssemblyEditorStyle::GetStyleSetName())
	{
	}

	//~TCommands
	virtual void RegisterCommands() override;
	//End TCommands

	/** Cell authoring: the bounds, hull and voxel tools plus the per-cell actions. */
	TSharedPtr<FUICommandInfo> LoadCellPalette;

	/** Junction authoring: the placement tool plus the per-junction actions. */
	TSharedPtr<FUICommandInfo> LoadJunctionPalette;

	/** World-scoped actions: the collision visualizer and the actor-ignore tagging toggles. */
	TSharedPtr<FUICommandInfo> LoadWorldPalette;

	/** Organ authoring: proxy generation and level-instance management. */
	TSharedPtr<FUICommandInfo> LoadOrganPalette;
};
