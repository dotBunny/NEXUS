// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Editor-utility-widget asset references used when opening the ProcGen tool panel.
 */
namespace NEXUS::ProcGenEditor::EditorUtilityWidget
{
	/** Content-browser path of the EUW asset that provides the ProcGen system UI. */
	inline FString Path = TEXT("/NexusProcGen/EditorResources/EUW_NProcGenSystem.EUW_NProcGenSystem");

	/** Tab identifier used when spawning the widget into the editor. */
	inline FName Identifier = TEXT("EUW_NProcGenSystem");

	/** Slate icon shown on the toolbar button that opens the widget. */
	inline FName Icon = TEXT("Icon.ProcGen");
}


/** Log category for the NexusProcGenEditor module. */
NEXUSPROCGENEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusProcGenEditor, Log, All);