// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorValidator.h"

#include "AssetDefinitions/AssetDefinition_NActorPoolSet.h"
#include "NActorPoolSet.h"

bool UNActorPoolsEditorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UNActorPoolSet>();
}

EDataValidationResult UNActorPoolsEditorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{

	if (InAsset->IsA<UNActorPoolSet>())
	{
		return UAssetDefinition_NActorPoolSet::ValidateAsset(InAssetData, InAsset, Context);
	}
	
	return  EDataValidationResult::NotValidated;
}