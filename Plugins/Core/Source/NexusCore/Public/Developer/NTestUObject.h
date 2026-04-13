// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NTestUObject.generated.h"

UCLASS(NotPlaceable, Hidden, HideDropdown, MinimalAPI)
class UNTestUObject : public UObject
{
	GENERATED_BODY()
	
public:
	int Counter;
	FString Message;
	uint8 State;
};
