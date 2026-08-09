 // Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Rails/NWorldAssemblyEdModeRail.h"

/**
 * The Cell Data category: authoring the focused cell's data asset
 *
 * Split into three groups — the interactive tools, the per-cell settings as checkboxes, and one-shot actions.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeCellDataRail final : public FNWorldAssemblyEdModeRail
{
public:
	using FNWorldAssemblyEdModeRail::FNWorldAssemblyEdModeRail;

	//~FNWorldAssemblyEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/**
	 * @return A predicate requiring a cell actor in the level.
	 * @note Every tool here refuses to build without one (UNCellToolBuilderBase::CanBuildTool) and every action needs
	 *       one to act on. Add Actor deliberately lives on the World rail, which is never hidden, so the cell that
	 *       brings this category back is still reachable while it is gone.
	 */
	virtual TAttribute<bool> GetAvailable() const override;

	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNWorldAssemblyEdModeRail
};
