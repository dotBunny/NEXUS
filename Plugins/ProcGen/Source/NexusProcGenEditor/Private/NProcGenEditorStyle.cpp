// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorStyle.h"

#include "NProcGenEdMode.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNProcGenEditorStyle)

FSlateIcon FNProcGenEditorStyle::CellActorEditHullModeIcon()
{
	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::ENCellEdMode::Hull)
	{
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Hull.Selected");
	}
	return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Hull");
}

FSlateIcon FNProcGenEditorStyle::CellActorEditBoundsModeIcon()
{
	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::ENCellEdMode::Bounds)
	{
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Bounds.Selected");
	}
	return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Bounds");
}

FSlateIcon FNProcGenEditorStyle::CellActorEditVoxelModeIcon()
{
	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::ENCellEdMode::Voxel)
	{
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Voxel.Points.Selected");
	}
	return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Voxel.Points");
}

FSlateIcon FNProcGenEditorStyle::CellActorToggleDrawVoxelDataIcon()
{
	switch (FNProcGenEdMode::GetCellVoxelMode())
	{
	case FNProcGenEdMode::ENCellVoxelMode::Grid:
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Voxel.Grid.Selected");
	case FNProcGenEdMode::ENCellVoxelMode::Points:
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Voxel.Points.Selected");
	default:
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.Voxel.Grid");
	}
}

FSlateIcon FNProcGenEditorStyle::CollisionVisualizerToggleIcon()
{
	if (FNProcGenEdMode::HasCollisionVisualizer())
	{
		return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.HasCollisionVisualizer");
	}
	return FSlateIcon(GetStyleSetName(), "Command.ProcGenEd.CreateCollisionVisualizer");
}

TSharedRef<FSlateStyleSet> FNProcGenEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE
	
	// NCellActor
	Style.Set("ClassIcon.NCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellActor"), Icon16x16));
	Style.Set("ClassThumbnail.NCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellActor"), Icon128x128));
	
	// NCellProxyActor
	Style.Set("ClassIcon.NCellProxy", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellProxy"), Icon16x16));
	Style.Set("ClassThumbnail.NCellProxy",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellProxy"), Icon128x128));

	// NCellLevelInstance
	Style.Set("ClassIcon.NCellLevelInstance", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellLevelInstance"), Icon16x16));
	Style.Set("ClassThumbnail.NCellLevelInstance",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellLevelInstance"), Icon128x128));
	
	// NCellRootComponent
	Style.Set("ClassIcon.NCellRootComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellRootComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NCellRootComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellRootComponent"), Icon128x128));
	
	// NCellJunctionComponent
	Style.Set("ClassIcon.NCellJunctionComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellJunctionComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NCellJunctionComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCellJunctionComponent"), Icon128x128));
	
	// NBoneComponent
	Style.Set("ClassIcon.NBoneComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NBoneComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NBoneComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NBoneComponent"), Icon128x128));
	Style.Set("ClassIcon.NBoneActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NBoneComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NBoneActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NBoneComponent"), Icon128x128));
	
	// NProcGenVolume/NProcGenComponent
	Style.Set("ClassIcon.NOrganComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NOrganVolume"), Icon16x16));
	Style.Set("ClassThumbnail.NOrganComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NOrganVolume"), Icon128x128));
	Style.Set("ClassIcon.NOrganVolume", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NOrganVolume"), Icon16x16));
	Style.Set("ClassThumbnail.NOrganVolume",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NOrganVolume"), Icon128x128));
	
	// NCell
	Style.Set("ClassIcon.NCell", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCell"), Icon16x16));
	Style.Set("ClassThumbnail.NCell",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCell"), Icon128x128));

	// NCellSet
	Style.Set("ClassIcon.NTissue", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NTissue"), Icon16x16));
	Style.Set("ClassThumbnail.NTissue",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NTissue"), Icon128x128));
	
	Style.Set("Icon.ProcGen", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Icon_ProcGen"), Icon128x128));
	
	Style.Set("Command.ProcGenEd.NCellLevelInstance", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_NCellLevelInstance"), Icon16x16));
	Style.Set("Command.ProcGenEd.NCellProxy", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_NCellProxy"), Icon16x16));
	Style.Set("Command.ProcGenEd.NOrganComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_NOrganComponent"), Icon16x16));

	Style.Set("Command.ProcGenEd.SelectNCellJunctionComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_SelectNCellJunctionComponent"), Icon16x16));
	Style.Set("Command.ProcGenEd.AddNCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_AddNCellActor"), Icon16x16));
	Style.Set("Command.ProcGenEd.SelectNCellActor.Selected", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_SelectNCellActor_Selected"), Icon16x16));
	Style.Set("Command.ProcGenEd.RemoveNCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_RemoveNCellActor"), Icon16x16));
	Style.Set("Command.ProcGenEd.Bounds", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Bounds"), Icon16x16));
	Style.Set("Command.ProcGenEd.Bounds.Selected", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Bounds_Selected"), Icon16x16));
	Style.Set("Command.ProcGenEd.CalculateBounds", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_CalculateBounds"), Icon16x16));
	Style.Set("Command.ProcGenEd.Hull", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Hull"), Icon16x16));
	Style.Set("Command.ProcGenEd.Hull.Selected", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Hull_Selected"), Icon16x16));
	Style.Set("Command.ProcGenEd.CalculateHull", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_CalculateHull"), Icon16x16));
	Style.Set("Command.ProcGenEd.CalculateVoxelData", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_CalculateVoxelData"), Icon16x16));
	Style.Set("Command.ProcGenEd.Voxel.Grid", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Voxel_Grid"), Icon16x16));
	Style.Set("Command.ProcGenEd.Voxel.Grid.Selected", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Voxel_Grid_Selected"), Icon16x16));
	Style.Set("Command.ProcGenEd.Voxel.Points", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Voxel_Points"), Icon16x16));
	Style.Set("Command.ProcGenEd.Voxel.Points.Selected", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Voxel_Points_Selected"), Icon16x16));
	
	Style.Set("Command.ProcGenEd.Pin", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_Pin"), Icon16x16));

	Style.Set("Command.ProcGenEd.HasCollisionVisualizer", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_HasCollisionVisualizer"), Icon16x16));
	Style.Set("Command.ProcGenEd.CreateCollisionVisualizer", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_EdMode_CreateCollisionVisualizer"), Icon16x16));

	
	return StyleRef;
}
