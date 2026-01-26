#include "NDynamicRefListViewEntry.h"

#include "CommonTextBlock.h"
#include "NColor.h"
#include "NDynamicRefObject.h"
#include "Components/NListView.h"
#include "Developer/NTextObject.h"

void UNDynamicRefListViewEntry::NativeDestruct()
{
	if (Object != nullptr)
	{
		Object->Changed.Unbind();
		Object = nullptr;
	}
	Super::NativeDestruct();
}

void UNDynamicRefListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Object = Cast<UNDynamicRefObject>(ListItemObject);
	Object->Changed.BindUObject(this, &UNDynamicRefListViewEntry::Refresh);
	
	Refresh();
}

void UNDynamicRefListViewEntry::NativeOnEntryReleased()
{
	if (Object != nullptr)
	{
		Object->Changed.Unbind();
		Object = nullptr;
	}
	
	INListViewEntry::NativeOnEntryReleased();
}

void UNDynamicRefListViewEntry::Refresh() const
{
	Reference->SetText(Object->GetReferenceText());
	if (Object != nullptr)
	{
		References->ClearListItems();
		
		for (const UObject* ReferenceObject : Object->GetObjects())
		{
			UNTextObject* TextObject = NewObject<UNTextObject>(
				Object, UNTextObject::StaticClass(), NAME_None, RF_Transient);
			
			TextObject->SetText(FText::FromName(ReferenceObject->GetFName()));
			TextObject->SetForegroundColor(ENColor::NC_White);
			
			References->AddItem(TextObject);
		}
	}
}