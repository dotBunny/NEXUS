// Copyright dotBunny Inc. All Rights Reserved.

#include "NToolingEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNToolingEditorStyle)

TSharedRef<FSlateStyleSet> FNToolingEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE
	
	Style.Set("Command.LeakCheck", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_LeakCheck"), Icon16x16));
	Style.Set("Command.CleanLogsFolder", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_CleanLogsFolder"), Icon16x16));
	Style.Set("Command.Visualizer", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Visualizer"), Icon16x16));
	Style.Set("Command.FindAndFix", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_FindAndFix"), Icon16x16));
	Style.Set("Command.FindAndFix.Item", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_FindAndFix_Item"), Icon16x16));
	Style.Set("Command.ProjectLevels", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_ProjectLevels"), Icon16x16));
	
	
	// NCollisionVisualizerActor / Window
	Style.Set("ClassIcon.NCollisionVisualizerActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCollisionVisualizerActor"), Icon16x16));
	Style.Set("ClassThumbnail.NCollisionVisualizerActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCollisionVisualizerActor"), Icon128x128));
	
	return StyleRef;
}