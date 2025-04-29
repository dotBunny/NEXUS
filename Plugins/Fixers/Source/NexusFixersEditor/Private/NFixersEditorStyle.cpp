// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNFixersEditorStyle)

TSharedRef<FSlateStyleSet> FNFixersEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE

	Style.Set("Command.FindAndFix", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_FindAndFix"), Icon16x16));
	Style.Set("Command.FindAndFix.Item", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_FindAndFix_Item"), Icon16x16));
	
	return StyleRef;
}