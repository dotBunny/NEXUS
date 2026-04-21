// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "Macros/NEditorStyleMacros.h"
#include "Templates/SharedPointer.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Slate style set for the NexusUI editor module. The N_IMPLEMENT_EDITOR_STYLE_HEADER macro
 * expands the boilerplate Get/Initialize/Shutdown helpers and wires the style against the
 * "NexusUI" plugin's `Resources/Slate` folder.
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class NEXUSUIEDITOR_API FNUIEditorStyle
{
	N_IMPLEMENT_EDITOR_STYLE_HEADER("NexusUI", "NUIEditorStyle")
};
