// Copyright dotBunny Inc. All Rights Reserved.

#include "NUIEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNUIEditorStyle)

TSharedRef<FSlateStyleSet> FNUIEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE

	Style.Set("Command.AddActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_AddActor"), Icon16x16));
	Style.Set("Command.Calculate", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Calculate"), Icon16x16));
	Style.Set("Command.Build", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Build"), Icon16x16));
	Style.Set("Command.RemoveActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_RemoveActor"), Icon16x16));
	Style.Set("Command.Reset", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Reset"), Icon16x16));
	Style.Set("Command.SelectActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_SelectActor"), Icon16x16));

	// NCollisionQueryTestActor / Window
	Style.Set("ClassIcon.NCollisionQueryTestActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCollisionQueryTestActor"), Icon16x16));
	Style.Set("ClassThumbnail.NCollisionQueryTestActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NCollisionQueryTestActor"), Icon128x128));
	
	return StyleRef;
}