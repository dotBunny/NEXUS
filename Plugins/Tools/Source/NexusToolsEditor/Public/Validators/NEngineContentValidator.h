// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NEngineContentValidator.generated.h"

class UK2Node;
class UK2Node_Event;

/**
 * A validator that will flag if the change is to engine-based content.
 * This attempts to notify/stop changes to packaged engine content by developers.
 */
UCLASS()
class UNEngineContentValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};