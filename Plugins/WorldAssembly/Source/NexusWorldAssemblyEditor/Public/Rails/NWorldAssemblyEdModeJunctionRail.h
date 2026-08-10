// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Rails/NWorldAssemblyEdModeRail.h"

/**
 * The Junction category: placing and managing the focused cell's junctions.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeJunctionRail final : public FNWorldAssemblyEdModeRail
{
public:
	using FNWorldAssemblyEdModeRail::FNWorldAssemblyEdModeRail;

	//~FNWorldAssemblyEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/** @return A predicate requiring a cell actor; a junction is a component of one, so nothing here works without it. */
	virtual TAttribute<bool> GetAvailable() const override;

	/**
	 * @return The tools, then the junction picker, then the component operations.
	 * @note No CreateHeader override, unlike the other categories with a picker: the panel puts a header above the
	 *       content, and this category wants its tool tiles first. Building the picker as part of the content is what
	 *       puts the ordering in the rail's hands, and it costs nothing — the panel drops both into the same scrolling
	 *       column anyway, so the picker only has to carry the inset the header slot would have given it.
	 */
	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNWorldAssemblyEdModeRail
};
