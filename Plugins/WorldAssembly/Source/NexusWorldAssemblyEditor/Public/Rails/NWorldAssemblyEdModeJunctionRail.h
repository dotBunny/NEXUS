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
	virtual TAttribute<bool> GetEnabled() const override;

	/** @return A combo box naming the selected junction, and listing the level's others to switch to. */
	virtual TSharedPtr<SWidget> CreateHeader() const override;

	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNWorldAssemblyEdModeRail
};
