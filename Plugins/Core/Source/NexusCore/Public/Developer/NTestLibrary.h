// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NTestCollection.h"
#include "NTestLibrary.generated.h"

UCLASS()
class UNTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	static UNTestCollection* CreateTestCollection() { return NewObject<UNTestCollection>(); }
};