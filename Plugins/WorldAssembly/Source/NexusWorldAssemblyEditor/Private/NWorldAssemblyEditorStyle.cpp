// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorStyle.h"

#include "NEditorUtils.h"
#include "NWorldAssemblyEditorQuickAssembly.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/ToolBarStyle.h"

N_EDITOR_STYLE(FNWorldAssemblyEditorStyle)

FSlateIcon FNWorldAssemblyEditorStyle::QuickAssemblyOperationIcon()
{
	// Show the cancel icon for the whole loop — both while an operation runs and during the wait between auto-runs.
	if (FNWorldAssemblyEditorQuickAssembly::IsActive())
	{
		return FSlateIcon(GetStyleSetName(), "Command.WorldAssemblyEd.CancelQuickAssemblyOperation");
	}
	return FSlateIcon(GetStyleSetName(), "Command.WorldAssemblyEd.QuickAssemblyOperation");
}

TSharedRef<FSlateStyleSet> FNWorldAssemblyEditorStyle::Create()
{
	N_EDITOR_STYLE_CREATE

	Style.Set("Icon.WorldAssembly", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Icon_WorldAssembly"), Icon128x128));
	Style.Set("Icon.WorldAssembly.Small", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Icon_WorldAssembly"), Icon16x16));

	// CLASS ICONS - Colored
	Style.Set("ClassIcon.NCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellActor"), Icon16x16));
	Style.Set("ClassThumbnail.NCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellActor"), Icon128x128));
	Style.Set("ClassIcon.NCellProxy", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellProxy"), Icon16x16));
	Style.Set("ClassThumbnail.NCellProxy",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellProxy"), Icon128x128));
	Style.Set("ClassIcon.NCellLevelInstance", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellLevelInstance"), Icon16x16));
	Style.Set("ClassThumbnail.NCellLevelInstance",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellLevelInstance"), Icon128x128));
	Style.Set("ClassIcon.NCellRootComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellRootComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NCellRootComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellRootComponent"), Icon128x128));
	Style.Set("ClassIcon.NCellJunctionComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellJunctionComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NCellJunctionComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCellJunctionComponent"), Icon128x128));
	Style.Set("ClassIcon.NBoneComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NBoneComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NBoneComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NBoneComponent"), Icon128x128));
	Style.Set("ClassIcon.NBoneActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NBoneComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NBoneActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NBoneComponent"), Icon128x128));
	Style.Set("ClassIcon.NOrganComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NOrganVolume"), Icon16x16));
	Style.Set("ClassThumbnail.NOrganComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NOrganVolume"), Icon128x128));
	Style.Set("ClassIcon.NOrganVolume", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NOrganVolume"), Icon16x16));
	Style.Set("ClassThumbnail.NOrganVolume",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NOrganVolume"), Icon128x128));
	Style.Set("AssetOverlay.NCell", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/AssetOverlay_NCell"), Icon16x16));
	Style.Set("ClassIcon.NCell", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCell"), Icon16x16));
	Style.Set("ClassThumbnail.NCell",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NCell"), Icon128x128));
	Style.Set("ClassIcon.NTissue", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NTissue"), Icon16x16));
	Style.Set("ClassThumbnail.NTissue",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Classes/ClassIcon_NTissue"), Icon128x128));

	// TOOLS
	Style.Set("Tool.CollisionVisualizer", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CollisionVisualizer"), Icon20x20));
	Style.Set("Tool.CellBounds", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CellBounds"), Icon20x20));
	Style.Set("Tool.CellHullSplitEdge", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CellHullSplitEdge"), Icon20x20));
	Style.Set("Tool.CellHullVertex", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CellHullVertex"), Icon20x20));
	Style.Set("Tool.CellVoxelPoint", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CellVoxelPoint"), Icon20x20));
	Style.Set("Tool.JunctionPlacement", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_JunctionPlacement"), Icon20x20));


	// Editor Mode Rebuild
	Style.Set("Rail.World.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_World"), Icon20x20));
	Style.Set("Rail.Cell.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_Cell"), Icon20x20));
	Style.Set("Rail.CellData.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_CellData"), Icon20x20));

	// JUNCTION RAIL
	Style.Set("Rail.Junction.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_Junction"), Icon20x20));
	Style.Set("Rail.Junction.CollectJunctions", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_CollectJunctions"), Icon16x16));

	// CELL RAIL
	Style.Set("Rail.Cell.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_Cell"), Icon20x20));
	Style.Set("Rail.Cell.CalculateCellBounds", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_CalculateCellBounds"), Icon20x20));
	Style.Set("Rail.Cell.CalculateCellHull", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_CalculateCellHull"), Icon20x20));
	Style.Set("Rail.Cell.CalculateCellVoxel", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_CalculateCellVoxel"), Icon20x20));

	// ORGAN RAIL
	Style.Set("Rail.Organ.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Rails/Rail_Organ"), Icon20x20));
	Style.Set("Rail.Organ.GenerateAllProxies", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_GenerateAllProxies"), Icon20x20));
	Style.Set("Rail.Organ.ClearAllProxies", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ClearAllProxies"), Icon20x20));
	Style.Set("Rail.Organ.LoadAllLevelInstances", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_LoadAllLevelInstances"), Icon20x20));
	Style.Set("Rail.Organ.UnloadAllLevelInstances", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_UnloadAllLevelInstances"), Icon20x20));
	Style.Set("Rail.Organ.GenerateProxies", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_GenerateAllProxies"), Icon20x20));
	Style.Set("Rail.Organ.ClearProxies", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ClearAllProxies"), Icon20x20));
	Style.Set("Rail.Organ.LoadLevelInstances", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_LoadAllLevelInstances"), Icon20x20));
	Style.Set("Rail.Organ.UnloadLevelInstances", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_UnloadAllLevelInstances"), Icon20x20));


	// QUICK ASSEMBLY

	Style.Set("Command.WorldAssemblyEd.QuickAssemblyOperation", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_QuickAssemblyOperation"), Icon128x128));
	Style.Set("Command.WorldAssemblyEd.CancelQuickAssemblyOperation", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_CancelQuickAssemblyOperation"), Icon128x128));

	// Rounded to sit inside the rounded box the panel itself is drawn on. A tighter radius than the 6 the two overlays
	// use, because a group nested inside a rounded container reads wrong at the same radius as its container — the
	// inner curve has to be the smaller one.
	const FSlateRoundedBoxBrush GroupBackground(FStyleColors::Recessed, 4.0f);

	// Behind CreateContent, and through it the tile palettes and the Organ rail's operations list. The button
	// groups gave their backing up and draw onto the panel, so this is no longer the frame every group shares — it is
	// what marks out the two kinds of section that are not a row of labelled buttons.
	Style.Set("WorldAssemblyEd.GroupBackground", new FSlateRoundedBoxBrush(GroupBackground));

	// The toolbars behind FNWorldAssemblyEdModeRail's CreateCommandGrid and CreateCommandList. Everything
	// about the labelled full-width buttons comes from the engine's SlimPaletteToolBar, which the toolkit builder
	// renders a palette with. The column count is the only difference between the two.

	// The icon tiles behind CreateCommandPalette. Read by SNCommandTile rather than by a toolbar, so only the
	// pieces a tile draws itself from are live here — ButtonStyle, ToggleButton, LabelStyle, IconPadding, LabelPadding.
	// Stock PaletteToolBar for all of them, which is what keeps a tile looking like the palette button it replaces.
	FToolBarStyle PaletteStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("PaletteToolBar");

	// Nothing reads it on the tile path — the group's well comes from CreateContent — but a style carrying a
	// different backing than the group it ends up in would be a trap for whatever puts these buttons in a toolbar next.
	PaletteStyle.SetBackground(GroupBackground);
	Style.Set("WorldAssemblyEd.CommandPalette", PaletteStyle);

	FToolBarStyle CommandStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("SlimPaletteToolBar");
	CommandStyle.SetBackground(FSlateNoResource());
	// Only the far side, because the buttons pad their own near side: SlimPaletteToolBar gives each one 4 on its left
	// and top and nothing on its right and bottom, so this is what makes the two sides read even.
	CommandStyle.SetBackgroundPadding(FMargin(0.0f, 0.0f, 4.0f, 4.0f));

	CommandStyle.SetNumColumns(2);
	Style.Set("WorldAssemblyEd.CommandGrid", CommandStyle);

	// The list is the grid at one column and without the buttons' resting fill, so a row is its icon and label until
	// the cursor is over it — the read Mesh Terrain's submode palettes have. SlimPaletteToolBar is what puts the fill
	// there: its ButtonStyle.Normal is a Dropdown-filled rounded box, unlike the plain SlimToolBar it is built from,
	// whose button comes from SimpleButton and paints nothing at rest. Clearing Normal alone gets it back — Hovered
	// and Pressed keep the rounded Hover box, which is the highlight this leaves behind.
	//
	// The grid keeps its fill on purpose: two buttons to a row need an edge each to read as two, and the same tiles
	// unfilled run together into one strip.
	FToolBarStyle CommandListStyle = CommandStyle;
	CommandListStyle.SetNumColumns(1);
	CommandListStyle.ButtonStyle.SetNormal(FSlateNoResource());

	// The toggle half of the same treatment, for a check-state command dropped into a list later. Only the unchecked
	// image goes — checked stays filled, which is the whole of how a toggle reports itself here.
	CommandListStyle.ToggleButton.SetUncheckedImage(FSlateNoResource());

	Style.Set("WorldAssemblyEd.CommandList", CommandListStyle);

	// The rule a rail can put between two groups. Box rather than an image so it stretches to whatever width the slot
	// gives it, and Hover rather than a border color because it has to read against the panel fill without drawing as
	// hard as an edge would — the same brush Mesh Terrain rules its tool panel sections with.
	Style.Set("WorldAssemblyEd.GroupSeparator", new FSlateColorBrush(FStyleColors::Hover));

	// The two boxes the edit mode floats over the viewport. A matched pair: the pinned category strip takes the
	// lighter fill and the panel it drives takes the darker one, which is the same relationship Mesh Terrain's
	// SubmodePaletteLighterBrush and SubmodePaletteDarkerBrush have. Rounded rather than the square ToolPanel.GroupBorder
	// they used to sit on, so they read as things floating over the scene rather than panels docked to nothing.
	Style.Set("WorldAssemblyEd.RailBackground",
		new FSlateRoundedBoxBrush(FStyleColors::Dropdown, 6.0f, FColor(85, 85, 85), 1.0f));
	Style.Set("WorldAssemblyEd.PanelBackground",
		new FSlateRoundedBoxBrush(FStyleColors::Panel, 6.0f, FColor(85, 85, 85), 1.0f));

	// The category strip's buttons. Icon-only comes from the engine's own no-label variant; what this changes is that
	// the toolbar stops painting a background of its own.
	FToolBarStyle CategoryToolBarStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("CategoryDrivenContentBuilderToolbarWithoutLabels");

	// Both cleared so the border behind the toolbar is the only thing drawing a background. Left alone, this style
	// inherits SlimToolBar's flat square FStyleColors::Panel fill, which paints a second color inside the rounded
	// backing above and squares its corners off — Slate does not clip a child to the rounding of the brush behind it.
	// The border's own padding supplies the inset the cleared BackgroundPadding used to.
	CategoryToolBarStyle.SetBackground(FSlateNoResource());
	CategoryToolBarStyle.SetBackgroundPadding(FMargin(0.0f));

	// Sized to Mesh Terrain's palette rather than the engine default, which is built for a rail wide enough to carry
	// labels under its icons. With the labels gone, that default leaves the highlight far larger than the icon inside
	// it: 8 units of icon padding on every side, and room reserved out to 36 across.
	//
	// The arithmetic is worth keeping straight, because these three have to agree — 20-unit icon plus 4 on each side
	// gives a 28-square button, which is what MaxWidth then has to allow through or the highlight is clipped narrower
	// than it is tall.
	CategoryToolBarStyle.SetIconPadding(FMargin(4.0f));
	CategoryToolBarStyle.SetButtonContentMaxWidth(28.0f);

	// Spacing around each button, and the middle term of the three that decide the strip's gutter: 7 from the border,
	// 2 here, and 4 of IconPadding, putting every icon 13 off the rounded edge on all four sides.
	//
	// Uniform, unlike the engine default, which pads vertically and not at all across — that left the icons sitting
	// 9 from the top and 4 from the sides, so the column read as a cramped ribbon. Worth measuring against the icon
	// rather than the highlight when tuning this, since the highlight sits 4 further out on every side and the two
	// give answers that differ by more than the numbers here do.
	//
	// Doubles up between neighbors, which with IconPadding is what puts 12 between one icon and the next.
	CategoryToolBarStyle.SetButtonPadding(FMargin(2.0f));

	// Not read on this path — see the rail icon brushes above — but left agreeing with them so the style is not
	// quietly claiming a size it does not produce.
	CategoryToolBarStyle.SetIconSize(FVector2D(20.0f, 20.0f));

	// Retuned for the lighter fill the strip now sits on. The inherited states assume a dark panel, so the unchecked
	// hover reads as muddy against Dropdown. These are the same four overrides Mesh Terrain applies to get a toggle
	// button onto its light palette; the checked states are already right and stay as they are.
	FCheckBoxStyle CategoryToggleButton = CategoryToolBarStyle.ToggleButton;
	CategoryToggleButton.SetUncheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::Hover, 4.0f));
	CategoryToggleButton.SetUncheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::DropdownOutline, 4.0f));
	CategoryToggleButton.SetHoveredForegroundColor(FStyleColors::ForegroundHover);
	CategoryToggleButton.SetPressedForegroundColor(FStyleColors::ForegroundHover);
	CategoryToolBarStyle.SetToggleButtonStyle(CategoryToggleButton);

	Style.Set("WorldAssemblyEd.CategoryToolBar", CategoryToolBarStyle);

	return StyleRef;
}
