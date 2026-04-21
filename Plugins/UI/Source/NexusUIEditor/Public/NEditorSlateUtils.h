// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A utility methods collection for editor-time Slate; primarily provides helpers to locate
 * SDockTab instances inside the editor's tab hierarchy so callers can reach tabs that were
 * spawned by other systems.
 */
class NEXUSUIEDITOR_API FNEditorSlateUtils
{
public:
	/**
	 * Walk the widget tree rooted at BaseWidget searching for an SDockTab that matches either
	 * TargetLabel or TabIdentifier.
	 * @param BaseWidget Root of the search; typically the parent tab or a known ancestor widget.
	 * @param TargetLabel Human-readable label to compare against the tab's displayed text.
	 * @param TabIdentifier Tab manager identifier to match; preferred over label when unique.
	 * @return The matching SDockTab, or an invalid shared pointer if nothing was found.
	 */
	static TSharedPtr<SDockTab> FindDocTab(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel, const FName TabIdentifier);

	/**
	 * Find an SDockTab anywhere in the global FGlobalTabmanager by identifier.
	 * @param TabIdentifier Tab manager identifier registered when the tab was spawned.
	 * @return The matching SDockTab, or an invalid shared pointer if no such tab is live.
	 */
	static TSharedPtr<SDockTab> FindDocTabByIdentifier(const FName TabIdentifier);
};

