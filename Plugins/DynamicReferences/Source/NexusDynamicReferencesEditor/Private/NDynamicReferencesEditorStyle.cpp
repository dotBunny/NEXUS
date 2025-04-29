// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferencesEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNDynamicReferencesEditorStyle)

TSharedRef<FSlateStyleSet> FNDynamicReferencesEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE
	
	// NDynamicReferenceComponent
	Style.Set("ClassIcon.NDynamicReferenceComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NDynamicReferenceComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NDynamicReferenceComponent",  new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NDynamicReferenceComponent"), Icon128x128));

	return StyleRef;
}