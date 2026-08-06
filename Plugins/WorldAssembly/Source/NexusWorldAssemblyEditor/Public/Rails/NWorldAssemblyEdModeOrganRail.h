// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Rails/NWorldAssemblyEdModeRail.h"

/**
 * The Organ category: generating an organ's cell proxies and managing their level instances.
 *
 * @note Every command here comes in a pair — one acting on the organ the header names, one on every organ in the
 *       level — so the palette is split along that line rather than by what the commands do.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeOrganRail final : public FNWorldAssemblyEdModeRail
{
public:
	using FNWorldAssemblyEdModeRail::FNWorldAssemblyEdModeRail;

	//~FNWorldAssemblyEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/** @return A predicate requiring an organ in the level; Add Organ lives on the World rail to bring this back. */
	virtual TAttribute<bool> GetEnabled() const override;

	/** @return A combo box naming the selected organ, and listing the level's others to switch to. */
	virtual TSharedPtr<SWidget> CreateHeader() const override;

	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNWorldAssemblyEdModeRail
};
