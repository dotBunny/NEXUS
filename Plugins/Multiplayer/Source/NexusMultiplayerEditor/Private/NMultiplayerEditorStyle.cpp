// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNMultiplayerEditorStyle)

TSharedRef<FSlateStyleSet> FNMultiplayerEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE

	Style.Set("Command.Multiplayer.StartMultiplayerTest.On", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_StartMultiplayerTest_On"), Icon16x16));
	Style.Set("Command.Multiplayer.StartMultiplayerTest.Off", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_StartMultiplayerTest_Off"), Icon16x16));
	Style.Set("Command.Multiplayer.StopMultiplayerTest.On", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_StopMultiplayerTest_On"), Icon16x16));
	Style.Set("Command.Multiplayer.StopMultiplayerTest.Off", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_StopMultiplayerTest_Off"), Icon16x16));
	
	// NTextRendererComponent
	Style.Set("ClassIcon.NTextRenderComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NTextRenderComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NTextRenderComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NTextRenderComponent"), Icon128x128));

	return StyleRef;
}