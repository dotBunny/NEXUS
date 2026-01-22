// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A utility methods collection for Slate
 */
class NEXUSUI_API FNSlateUtils
{

public:
	/**
	 * Attempt to find a SWidget by type within a parent widget.
	 * @param ParentWidget The parent widget to search within.
	 * @param WidgetType The type of widget to find.
	 * @return The found widget or nullptr if not found.
	 */
	static TSharedPtr<SWidget> FindWidgetByType(TSharedPtr<SWidget> ParentWidget, const FName& WidgetType);

	/**
	 * Attempt to find the SDocTab for a given SWidget.
	 * @param BaseWidget The SWidget to search upward from.
	 * @return The desired SDocTab or nullptr if not found
	 */
	static TSharedPtr<SDockTab> FindDocTab(const TSharedPtr<SWidget>& BaseWidget);

private:
	static const FName SDockingTabStackName;
	static const FName SDockTabName;
};
