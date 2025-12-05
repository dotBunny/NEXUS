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

class INListViewEntry : public IUserObjectListEntry
{
	GENERATED_BODY()
public:	
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner)
	{
		Execute_OnSetOwnerListView(Widget, Owner);
	};
	
	UFUNCTION(BlueprintImplementableEvent)
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void OnSetOwnerListView(UNListView* Owner);
};
