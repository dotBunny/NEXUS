// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NEditorStyleMacros.h"
#include "Textures/SlateIcon.h" // FSlateIcon return types declared below

/**
 * Slate style set for the NexusWorldAssemblyEditor module.
 */
class FNWorldAssemblyEditorStyle
{
	N_EDITOR_STYLE_HEADER("NexusWorldAssembly", "NWorldAssemblyEditorStyle")

public:
	/**
	 * @return Cancel icon while the tracked Quick Assembly operation is running, otherwise the start icon.
	 * @note The one state-dependent icon left. Every other command carries a fixed FSlateIcon on its FUICommandInfo,
	 *       which is what a toolbar built from a command reads; this one is reachable because the Quick Assembly
	 *       toolbar entry is an FToolMenuEntry that takes a TAttribute<FSlateIcon> directly.
	 */
	static FSlateIcon QuickAssemblyOperationIcon();
};