#include "NDynamicRefListViewEntry.h"

#include "CommonTextBlock.h"
#include "NDynamicRefObject.h"
#include "Components/NListView.h"
#include "Widgets/NButtonListViewEntry.h"
#include "Widgets/NTextListViewEntry.h"


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
			UNTextListViewEntryObject* TextObject = NewObject<UNTextListViewEntryObject>(
				Object, UNTextListViewEntryObject::StaticClass(), NAME_None, RF_Transient);

			TextObject->SetText(FText::FromString(FString::Printf(TEXT("- %s"), *ReferenceObject->GetFName().ToString())));
			

			References->AddItem(TextObject);
		}
	}
}