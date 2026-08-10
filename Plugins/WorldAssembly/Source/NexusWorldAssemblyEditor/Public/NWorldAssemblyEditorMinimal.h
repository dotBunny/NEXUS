// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/** Log category for the NexusWorldAssemblyEditor module. */
NEXUSWORLDASSEMBLYEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusWorldAssemblyEditor, Log, All);

/**
 * Sizes the edit mode's viewport handles are drawn at, in pixels.
 *
 * Shared because the cell tools and the cell-root visualizer draw the same geometry and have to agree: a hull vertex
 * has to look identical whether the visualizer drew it for a selected actor or a tool drew it for the focused cell,
 * and the split tool's marker has to read as the same kind of thing as the vertex tool's selection. That agreement
 * used to be four private constants kept in step by comments naming each other.
 */
namespace NEXUS::WorldAssembly::EdModeMetrics
{
	/**
	 * An ordinary, unselected handle.
	 * @note Also lands near the pick radius UNCellToolBase::IsPickedByRay resolves to on a typical viewport, which is
	 *       what makes the dot a fair advertisement of what clicking near it will hit.
	 */
	inline constexpr float HandleSize = 12.0f;

	/** A handle the tool is acting on — a selected vertex, or the point a click would split an edge at. */
	inline constexpr float ActiveHandleSize = 16.0f;

	/** Line thickness of a highlighted edge. */
	inline constexpr float HighlightThickness = 4.0f;
}