// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NTextObject.generated.h"

UCLASS(BlueprintType)
class NEXUSCORE_API UNTextObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetText(const FText& InText) { Text = InText; }
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	FText GetText() const { return Text; }
	
private:
	FText Text;
};