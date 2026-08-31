// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorStyle.h"

#include "NWorldAssemblyEditorQuickAssembly.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"

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
	Style.Set("ClassIcon.NWorldCollisionCacheActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CollisionVisualizer"), Icon16x16));
	Style.Set("ClassThumbnail.NWorldCollisionCacheActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Tools/Tool_CollisionVisualizer"), Icon128x128));

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

	// WORLD RAIL
	Style.Set("Command.BakeWorldCollision", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_BakeWorldCollision"), Icon20x20));


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

	return StyleRef;
}
