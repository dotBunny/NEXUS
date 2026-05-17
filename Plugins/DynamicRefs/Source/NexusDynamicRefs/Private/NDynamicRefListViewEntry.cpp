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
	if (Object != nullptr)
	{
		Object->Changed.BindUObject(this, &UNDynamicRefListViewEntry::Refresh);
	}
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
	if (!IsValid(Object)) return;
	const UNDynamicRefsDeveloperOverlay* Overlay = Object->GetOverlay();
	if (!IsValid(Overlay)) return;
	Overlay->OnButtonClicked.Broadcast(TargetObject);
}

void UNDynamicRefListViewEntry::Refresh() const
{
	if (IsValid(Object))
	{
		Reference->SetText(Object->GetReferenceText());
		
		// Remake buttons for referenced objects
		References->ClearListItems();
		
		for (TSoftObjectPtr ReferenceObject : Object->GetObjects())
		{
			UObject* ReferencePtr = ReferenceObject.Get();
			if (IsValid(ReferencePtr))
			{
				UNButtonListViewEntryObject* ButtonObject = NewObject<UNButtonListViewEntryObject>(
					Object, UNButtonListViewEntryObject::StaticClass(), NAME_None, RF_Transient);
			
				ButtonObject->SetText(FText::FromName(ReferenceObject->GetFName()));
				ButtonObject->TargetObject = ReferencePtr;
				ButtonObject->OnPressedEvent.BindUObject(this, &UNDynamicRefListViewEntry::OnButtonPressed);

				References->AddItem(ButtonObject);
			}
		}
	}
}
