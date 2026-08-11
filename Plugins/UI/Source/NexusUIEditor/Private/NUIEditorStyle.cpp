// Copyright dotBunny Inc. All Rights Reserved.

#include "NUIEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_EDITOR_STYLE(FNUIEditorStyle)

TSharedRef<FSlateStyleSet> FNUIEditorStyle::Create()
{
	N_EDITOR_STYLE_CREATE

	Style.Set("Command.AddActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_AddActor"), Icon16x16));
	Style.Set("Command.Build", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Build"), Icon16x16));
	Style.Set("Command.RemoveActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_RemoveActor"), Icon16x16));
	Style.Set("Command.Reset", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Reset"), Icon16x16));
	Style.Set("Command.SelectActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_SelectActor"), Icon16x16));

	Style.Set("Command.Select", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Select"), Icon20x20));
	Style.Set("Command.Tag", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Tag"), Icon20x20));
	Style.Set("Command.TagRemove", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_TagRemove"), Icon20x20));
	Style.Set("Command.Calculate", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Calculate"), Icon20x20));
	Style.Set("Command.ToggleOn", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ToggleOn"), Icon20x20));
	Style.Set("Command.ToggleOff", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ToggleOff"), Icon20x20));


	return StyleRef;
}