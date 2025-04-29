// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"

N_IMPLEMENT_EDITOR_STYLE(FNActorPoolsEditorStyle)

TSharedRef<FSlateStyleSet> FNActorPoolsEditorStyle::Create()
{
	N_IMPLEMENT_EDITOR_STYLE_CREATE


	// NPooledActor
	Style.Set("ClassIcon.NPooledActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NPooledActor"), Icon16x16));
	Style.Set("ClassThumbnail.NCellActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NPooledActor"), Icon128x128));

	// NActorPool
	Style.Set("ClassIcon.NActorPool", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPool"), Icon16x16));
	Style.Set("ClassThumbnail.NActorPool", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPool"), Icon128x128));

	// NActorPoolSet
	Style.Set("ClassIcon.NActorPoolSet", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPoolSet"), Icon16x16));
	Style.Set("ClassThumbnail.NActorPoolSet", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPoolSet"), Icon128x128));

	// NActorPoolSpawnerComponent
	Style.Set("ClassIcon.NActorPoolSpawnerComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPoolSpawnerComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NActorPoolSpawnerComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NActorPoolSpawnerComponent"), Icon128x128));

	// NKillZoneComponent
	Style.Set("ClassIcon.NKillZoneComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NKillZoneComponent"), Icon16x16));
	Style.Set("ClassThumbnail.NKillZoneComponent", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("ClassIcon_NKillZoneComponent"), Icon128x128));
	
	return StyleRef;
}