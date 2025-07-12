// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "AssetDefinitionDefault.h"
#include "NActorPoolSet.h"
#include "NColor.h"
#include "AssetDefinition_NActorPoolSet.generated.h"

/**
 * Asset definition for NActorPoolSet.
 */
UCLASS()
class UAssetDefinition_NActorPoolSet : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	//~ Begin UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FNColor::GetLinearColor(NC_NexusLightBlue); };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNActorPoolSet::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	//~ End UAssetDefinition

	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};