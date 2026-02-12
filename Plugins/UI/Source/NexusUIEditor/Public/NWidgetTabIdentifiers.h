// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWidgetTabIdentifiers.generated.h"

USTRUCT(BlueprintType)
struct FNWidgetTabIdentifiers
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FName> WidgetIdentifiers;
	
	UPROPERTY()
	TArray<FName> TabIdentifier;
};