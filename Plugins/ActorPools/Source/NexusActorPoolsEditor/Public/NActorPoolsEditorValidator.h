// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NActorPoolsEditorValidator.generated.h"

/**
 * Data validator that checks NexusActorPools assets (e.g. UNActorPoolSet) on save / commandlet validation.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/editor-types/actor-pools-editor-validator/">UNActorPoolsEditorValidator</a>
 */
UCLASS()
class UNActorPoolsEditorValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};