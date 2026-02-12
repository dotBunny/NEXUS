// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSlateUtils.h"

const FName FNSlateUtils::SDockingTabStackName = FName("SDockingTabStack");
const FName FNSlateUtils::SDockTabName = FName("SDockTab");

TSharedPtr<SWidget> FNSlateUtils::FindFirstWidgetByType(TSharedPtr<SWidget> ParentWidget, const FName& WidgetType)
{
	if (!ParentWidget.IsValid())
	{
		return nullptr;
	}
	
	if (ParentWidget->GetType() == WidgetType)
	{
		return ParentWidget;
	}
	
	FChildren* Children = ParentWidget->GetChildren();
	for (int i = 0; i < Children->Num(); ++i)
	{
		TSharedPtr<SWidget> Found = FindFirstWidgetByType(Children->GetChildAt(i), WidgetType);
		if (Found.IsValid())
		{
			return Found;
		}
	}
	
	return nullptr;
}

void FNSlateUtils::FindWidgetsByType(TArray<TSharedPtr<SWidget>>& OutWidgets, TSharedPtr<SWidget> ParentWidget, const FName& WidgetType, const FName& WidgetTypeStop)
{
	if (!ParentWidget.IsValid() || (WidgetTypeStop != NAME_None && ParentWidget->GetType() == WidgetTypeStop))
	{
		return;
	}
	
	if (ParentWidget->GetType() == WidgetType)
	{
		OutWidgets.Add(ParentWidget);
		return;

	}
	
	FChildren* Children = ParentWidget->GetChildren();
	for (int i = 0; i < Children->Num(); ++i)
	{
		FindWidgetsByType(OutWidgets, Children->GetChildAt(i), WidgetType, WidgetTypeStop);
	}
}

// #SONARQUBE-DISABLE-CPP_S134
TSharedPtr<SDockTab> FNSlateUtils::FindDocTabWithLabel(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel)
{
	TSharedPtr<SWidget> Widget = BaseWidget;
	while (Widget.IsValid())
	{
		// Bound Tab
		if (Widget->GetType() == SDockingTabStackName)
		{
			FChildren* Children = Widget->GetChildren();
			int ChildrenCount = Children->Num();
			
			for (int i = 0; i < ChildrenCount; ++i)
			{
				const TSharedPtr<SWidget> ChildWidget = Children->GetChildAt(i);
				TArray<TSharedPtr<SWidget>> FoundWidgets;
				FindWidgetsByType(FoundWidgets, ChildWidget, SDockTabName, SDockingTabStackName);
				for (int j = 0; j < FoundWidgets.Num(); j++)
				{
					if (FoundWidgets[j].IsValid())
					{
						TSharedPtr<SDockTab> Tab = StaticCastSharedPtr<SDockTab>(FoundWidgets[j]);
						if (Tab->GetTabLabel().EqualTo(TargetLabel))
						{
							return StaticCastSharedPtr<SDockTab>(FoundWidgets[j]);
						}
					}
				}
			}
			
			// Not going to go up at this point cause we already hit the container above me
			return nullptr;
		}
		
		// Floating Tab  ?
		if (Widget->GetType() == SDockTabName)
		{
			return StaticCastSharedPtr<SDockTab>(Widget);
		}
		
		// Goes up
		Widget = Widget->GetParentWidget();
	}
	
	// So if we've reached here, we are probably not actually a visible tab.
	return nullptr;
}
// #SONARQUBE-ENABLE
