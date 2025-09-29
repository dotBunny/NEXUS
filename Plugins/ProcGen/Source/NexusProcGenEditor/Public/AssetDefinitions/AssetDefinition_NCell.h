// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "AssetDefinitionDefault.h"
#include "NColor.h"
#include "Cell/NCell.h"
#include "AssetDefinition_NCell.generated.h"

/**
 * Asset definition for NProceduralPieceSet.
 */
UCLASS()
class UAssetDefinition_NCell : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	FORCEINLINE static FString GetCellPackagePath(const FString& BasePath)
	{
		return BasePath + TEXT("_NCell");
	}
	static UNCell* GetOrCreatePackage(UWorld* World);
	
	static void OnAssetRemoved(const FAssetData& AssetData);
	static void OnAssetRenamed(const FAssetData& AssetData, const FString& String);
	static void OnPreSaveWorldWithContext(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext);

	//~ Begin UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FNColor::NexusLightBlue; };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNCell::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	// We do not want NCells to be duplicated as they are tied to a specific world.
	virtual FAssetSupportResponse CanDuplicate(const FAssetData& InAsset) const override { return FAssetSupportResponse::NotSupported(); }
	//~ End UAssetDefinition

	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};