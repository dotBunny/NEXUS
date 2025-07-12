﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "BlueprintValidator.generated.h"

class UK2Node;
class UK2Node_Event;

UCLASS()
class UBlueprintValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

	static bool IsEmptyTick(const UK2Node_Event* EventNode);
	static bool IsMultiPinPureNode(UK2Node* PureNode);
};