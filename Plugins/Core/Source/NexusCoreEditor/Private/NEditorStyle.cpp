// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNEditorStyle)

TSharedRef<FSlateStyleSet> FNEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE

	Style.Set("NEXUS.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("NEXUS"), Icon16x16));
	Style.Set("Command.LeakCheck", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_LeakCheck"), Icon16x16));
	Style.Set("Command.ProjectLevels", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_ProjectLevels"), Icon16x16));
	Style.Set("Command.Visualizer", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Visualizer"), Icon16x16));
	Style.Set("Command.OpenDiscordInviteLink", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_OpenDiscordInviteLink"), Icon16x16));
	
	return StyleRef;
}