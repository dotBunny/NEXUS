#include "NSlateUtils.h"

const FName FNSlateUtils::SDockingTabStackName = FName("SDockingTabStack");
const FName FNSlateUtils::SDockTabName = FName("SDockTab");

TSharedPtr<SWidget> FNSlateUtils::FindWidgetByType(TSharedPtr<SWidget> ParentWidget, const FName& WidgetType)
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
		TSharedPtr<SWidget> Found = FindWidgetByType(Children->GetChildAt(i), WidgetType);
		if (Found.IsValid())
		{
			return Found;
		}
	}
	
	return nullptr;
}

// #SONARQUBE-DISABLE-CPP_S134
TSharedPtr<SDockTab> FNSlateUtils::FindDocTab(const TSharedPtr<SWidget>& BaseWidget)
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
				TSharedPtr<SWidget> FoundWidget = FindWidgetByType(ChildWidget, SDockTabName);
				
				if (FoundWidget.IsValid())
				{
					return StaticCastSharedPtr<SDockTab>(FoundWidget);
				}
			}
		}
		
		// Floating Tab  ?
		if (Widget->GetType() == SDockTabName)
		{
			return StaticCastSharedPtr<SDockTab>(Widget);
		}
		
		Widget = Widget->GetParentWidget();
	}
	return nullptr;
}
// #SONARQUBE-ENABLE
