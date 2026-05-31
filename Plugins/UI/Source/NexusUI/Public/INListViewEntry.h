// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "INListViewEntry.generated.h"

class UNListView;

UINTERFACE(BlueprintType)
class NEXUSUI_API UNListViewEntry : public UUserObjectListEntry
{
	GENERATED_BODY()
};

/**
 * Extends UUserObjectListEntry so entries hosted in a UNListView are handed back a typed pointer
 * to their owning list at construction. Useful for entries that need to talk to sibling rows or
 * ask the list for selection state.
 */
class NEXUSUI_API INListViewEntry : public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	/** Called by UNListView when the entry widget is created; forwards to OnSetOwnerListView. */
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner)
	{
		Execute_OnSetOwnerListView(Widget, Owner);
	};

	/** Blueprint hook invoked with the owning UNListView; implement to cache the reference. */
	UFUNCTION(BlueprintImplementableEvent)
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void OnSetOwnerListView(UNListView* Owner);
};
