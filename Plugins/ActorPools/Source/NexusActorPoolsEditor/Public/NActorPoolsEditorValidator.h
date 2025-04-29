// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NActorPoolsEditorValidator.generated.h"

/**
 * A generalized validator for sub-validators.
 */
UCLASS()
class UNActorPoolsEditorValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};