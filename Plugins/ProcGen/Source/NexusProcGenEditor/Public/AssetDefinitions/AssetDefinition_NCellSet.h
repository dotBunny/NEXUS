﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "AssetDefinitionDefault.h"
#include "NColor.h"
#include "Cell/NCellSet.h"
#include "Macros/NEditorAssetMacros.h"
#include "AssetDefinition_NCellSet.generated.h"

/**
 * Asset definition for NCellSet.
 */
UCLASS()
class UAssetDefinition_NCellSet : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:

	//~ Begin UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FNColor::NexusLightBlue; };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNCellSet::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	//~ End UAssetDefinition

	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};


/**
 * Asset factory for NCellSet.
 */
UCLASS(MinimalAPI, HideCategories = Object)
class UNCellSetFactory : public UFactory
{
	GENERATED_BODY()
	explicit UNCellSetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		SupportedClass = UNCellSet::StaticClass();
		bCreateNew = true;
		bEditorImport = false;
		bEditAfterNew = false;
	};
	N_IMPLEMENT_ASSET_FACTORY(UNCellSet)
};