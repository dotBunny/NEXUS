#include "NDynamicRefListViewEntry.h"

void UNDynamicRefListViewEntry::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNDynamicRefListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
}

void UNDynamicRefListViewEntry::Refresh() const
{
}

void UNDynamicRefListViewEntry::Reset() const
{
}
