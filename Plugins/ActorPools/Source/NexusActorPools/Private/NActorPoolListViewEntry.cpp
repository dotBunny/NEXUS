#include "NActorPoolListViewEntry.h"

void UNActorPoolListViewEntry::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNActorPoolListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
}
