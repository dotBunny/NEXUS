// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NLevelBlueprintValidator.generated.h"

class UK2Node;
class UK2Node_Event;

/**
 * A validator that alerts to usage of Level Blueprint logic.
 */
UCLASS()
class UNLevelBlueprintValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};