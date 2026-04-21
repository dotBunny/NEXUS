// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Shared constants for NexusUI widgets surfaced in the widget designer palette.
 */
namespace NEXUS::UIEditor
{
	/** Palette category used by every NexusUI widget to group them together in the designer. */
	const FText PaletteCategory = NSLOCTEXT("NexusUIEditor", "WidgetPaletteCategory", "NEXUS UI");
}

/** Log category for the NexusUI runtime logging. */
NEXUSUI_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusUI, Log, All);