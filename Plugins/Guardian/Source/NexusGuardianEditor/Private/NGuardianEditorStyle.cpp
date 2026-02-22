// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianEditorStyle.h"
#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNGuardianEditorStyle)

TSharedRef<FSlateStyleSet> FNGuardianEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE
	
	Style.Set("Icon.NGuardian", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NGuardianSubsystem"), Icon16x16));
	
	return StyleRef;
}