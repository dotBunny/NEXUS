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
	virtual FLinearColor GetAssetColor() const override { return FNColor::GetLinearColor(ENColor::NC_NexusLightBlue); };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNActorPoolSet::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	//~ End UAssetDefinition

	/**
	 * Validate a single UNActorPoolSet asset, reporting issues to the provided context.
	 * @param InAssetData Registry-level metadata for the asset.
	 * @param InAsset The loaded asset instance.
	 * @param Context Validation context to accumulate messages into.
	 * @return The resulting validation verdict.
	 */
	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};