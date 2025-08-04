// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSharedSamplesEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNSharedSamplesEditorStyle)

TSharedRef<FSlateStyleSet> FNSharedSamplesEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE
	
	// NSamplesDisplayActor
	Style.Set("ClassIcon.NSamplesDisplayActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesDisplayActor"), Icon16x16));
	Style.Set("ClassThumbnail.NSamplesDisplayActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesDisplayActor"), Icon128x128));

	// NSamplesDisplayActor
	Style.Set("ClassIcon.NSamplesLevelActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesLevelActor"), Icon16x16));
	Style.Set("ClassThumbnail.NSamplesLevelActor",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesLevelActor"), Icon128x128));

	// NSamplesUserWidget
	Style.Set("ClassIcon.NSamplesUserWidget", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesUserWidget"), Icon16x16));
	Style.Set("ClassThumbnail.NSamplesUserWidget",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NSamplesUserWidget"), Icon128x128));
	
	return StyleRef;
}