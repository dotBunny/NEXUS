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
	 * Attempt to find the first SWidget by type within a parent widget.
	 * @param ParentWidget The parent widget to search within.
	 * @param WidgetType The type of widget to find.
	 * @return The found widget or nullptr if not found.
	 */
	static TSharedPtr<SWidget> FindFirstWidgetByType(TSharedPtr<SWidget> ParentWidget, const FName& WidgetType);
	
	/**
	 * Attempt to find the SWidgets by type within a parent widget.
	 * @param OutWidgets
	 * @param ParentWidget The parent widget to search within.
	 * @param WidgetType The type of widget to find.
	 * @param WidgetTypeStop
	 * @return The found widgets.
	 */
	static void FindWidgetsByType(TArray<TSharedPtr<SWidget>>& OutWidgets, TSharedPtr<SWidget> ParentWidget, const FName& WidgetType, const FName& WidgetTypeStop = NAME_None);
	
	
	/**
	 * Attempt to find the SDocTab for a given SWidget.
	 * @param BaseWidget The SWidget to search upward from.
	 * @param TargetLabel The label to validate the tab
	 * @return The desired SDocTab or nullptr if not found
	 */
	static TSharedPtr<SDockTab> FindDocTabWithLabel(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel);

private:
	static const FName SDockingTabStackName;
	static const FName SDockTabName;
};
