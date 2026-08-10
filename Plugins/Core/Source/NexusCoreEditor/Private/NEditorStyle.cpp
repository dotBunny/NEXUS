// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_EDITOR_STYLE(FNEditorStyle)

TSharedRef<FSlateStyleSet> FNEditorStyle::Create()
{
	N_EDITOR_STYLE_CREATE

	Style.Set("NEXUS.Icon", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("NEXUS"), Icon16x16));
	Style.Set("Lock", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Lock"), Icon16x16));
	Style.Set("Lock.Desaturated", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Lock_Desaturated"), Icon16x16));
	Style.Set("Command.OpenDiscordInviteLink", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_OpenDiscordInviteLink"), Icon16x16));

	// NDebugActor
	Style.Set("ClassIcon.NDebugActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NDebugActor"), Icon16x16));
	Style.Set("ClassThumbnail.NDebugActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NDebugActor"), Icon128x128));



	Style.Set("Command.Select", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Select"), Icon20x20));
	Style.Set("Command.Tag", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Tag"), Icon20x20));
	Style.Set("Command.TagRemove", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_TagRemove"), Icon20x20));

	return StyleRef;
}