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

void UNDynamicRefListViewEntry::OnButtonPressed(UObject* TargetObject) const
{
	Object->GetOverlay()->OnButtonClicked.Broadcast(TargetObject);
}

void UNDynamicRefListViewEntry::Refresh() const
{
	Reference->SetText(Object->GetReferenceText());
	if (Object != nullptr)
	{
		// Remake buttons for referenced objects
		References->ClearListItems();
		
		for (UObject* ReferenceObject : Object->GetObjects())
		{
			UNButtonListViewEntryObject* ButtonObject = NewObject<UNButtonListViewEntryObject>(
				Object, UNButtonListViewEntryObject::StaticClass(), NAME_None, RF_Transient);
			
			ButtonObject->SetText(FText::FromString(FString::Printf(TEXT("%s"), *ReferenceObject->GetFName().ToString())));
			ButtonObject->TargetObject = ReferenceObject;
			ButtonObject->OnPressedEvent.BindUObject(this, &UNDynamicRefListViewEntry::OnButtonPressed);

			References->AddItem(ButtonObject);
		}
	}
}
