// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NProcGenEditorValidator.generated.h"

/**
 * Editor data validator that dispatches into ProcGen-specific sub-validators based on asset type.
 *
 * Currently covers world assets (ensures cell-actor/junction data matches the side-car); extend
 * by adding further ValidateXxxAsset helpers and routing to them from ValidateLoadedAsset_Implementation.
 */
UCLASS()
class UNProcGenEditorValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
private:
	/** World-asset sub-validator — checks ProcGen cell/junction consistency for level assets. */
	static EDataValidationResult ValidateWorldAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};