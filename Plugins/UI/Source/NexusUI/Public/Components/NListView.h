// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonListView.h"
#include "INListViewEntry.h"
#include "NUINamespace.h"
#include "NListView.generated.h"

UCLASS(DisplayName = "ListView", ClassGroup = UI, meta = (Category = "NEXUS"))
/**
 * A wrapper around the UComboListView class to allow for calls to an interface on the entry elements. 
 **/
class NEXUSUI_API UNListView : public UCommonListView
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override {  return NEXUS::UI::Editor::PaletteCategory; }
#endif // WITH_EDITOR
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetReferenceObject(UObject* Object) { ReferenceObject = Object; }

	UFUNCTION(BlueprintCallable)
	UObject* GetReferenceObject() const
	{
		if (ReferenceObject.IsValid())
		{
			return ReferenceObject.Get();
		}
		return nullptr;
	}

protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override
	{
		UUserWidget& Widget =  Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
		
		if (Widget.Implements<UNListViewEntry>())
		{
			if (INListViewEntry* Entry = Cast<INListViewEntry>(&Widget); 
				Entry != nullptr)
			{
				Entry->SetOwnerListView(&Widget, this);
			}
			else
			{
				INListViewEntry::Execute_OnSetOwnerListView(&Widget, this);
			}
		}
		return Widget;
	};

	UPROPERTY(BlueprintreadOnly, Category = "NListView")
	TWeakObjectPtr<UObject> ReferenceObject = nullptr;
};
