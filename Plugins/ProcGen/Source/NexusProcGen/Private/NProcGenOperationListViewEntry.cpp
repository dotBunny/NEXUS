#include "NProcGenOperationListViewEntry.h"

#include "CommonTextBlock.h"
#include "NProcGenOperation.h"

void UNProcGenOperationListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UNProcGenOperation* Operation = Cast<UNProcGenOperation>(ListItemObject))
	{
		LeftText->SetText(FText::FromString(Operation->GetName()));
	}
}
