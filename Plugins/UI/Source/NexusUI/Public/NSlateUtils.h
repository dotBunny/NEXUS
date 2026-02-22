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
	 * Attempts to find the first SWidget by type within a parent widget and its children.
	 * @param ParentWidget The parent widget to search within.
	 * @param WidgetType The type of widget to find.
	 * @return The found widget or nullptr if not found.
	 */
	static TSharedPtr<SWidget> FindFirstWidgetByType(TSharedPtr<SWidget> ParentWidget, const FName& WidgetType);
	
	/**
	 * Attempts to find any SWidgets by type within a parent widget and its children.
	 * @param OutWidgets The found widgets array.
	 * @param ParentWidget The parent widget to search within.
	 * @param WidgetType The type of widget to find.
	 * @param WidgetTypeStop The type of widget to stop recursively searching at.
	 */
	static void FindWidgetsByType(TArray<TSharedPtr<SWidget>>& OutWidgets, TSharedPtr<SWidget> ParentWidget, const FName& WidgetType, const FName& WidgetTypeStop = NAME_None);
	
	/**
	 * Attempt to find the SDocTab for a given SWidget.
	 * @param BaseWidget The SWidget to search upward from.
	 * @param TargetLabel The label is used to ensure we are finding the correct tab.
	 * @return The desired SDocTab or nullptr if not found
	 */
	static TSharedPtr<SDockTab> FindDocTabWithLabel(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel);

private:
	static const FName SDockingTabStackName;
	static const FName SDockTabName;
};
