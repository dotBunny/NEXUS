// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "Macros/NEditorStyleMacros.h"

/**
 * Slate style set for the NexusUI editor module. The N_EDITOR_STYLE_HEADER macro
 * expands the boilerplate Get/Initialize/Shutdown helpers and wires the style against the
 * "NexusUI" plugin's `Resources/Slate` folder.
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class NEXUSUIEDITOR_API FNUIEditorStyle
{
	N_EDITOR_STYLE_HEADER("NexusUI", "NUIEditorStyle")
};
