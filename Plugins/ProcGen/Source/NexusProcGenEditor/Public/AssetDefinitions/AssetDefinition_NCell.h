// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "AssetDefinitionDefault.h"
#include "NColor.h"
#include "Cell/NCell.h"
#include "AssetDefinition_NCell.generated.h"

/**
 * Asset definition for UNCell — the on-disk side-car that mirrors a cell actor's authored bounds,
 * hull, and voxel data. Also owns the package-lifecycle hooks that keep the side-car asset in sync
 * with its host world (rename/remove/save).
 *
 * Duplication is disabled because a cell side-car is tied to a specific world and a specific cell
 * actor; copying the asset without rebinding would produce a phantom reference.
 */
UCLASS()
class UAssetDefinition_NCell : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	/** @return the canonical side-car package path for the given world/base path. */
	FORCEINLINE static FString GetCellPackagePath(const FString& BasePath)
	{
		return BasePath + TEXT("_NCell");
	}

	/** @return the existing side-car asset for World, creating a new one on disk if missing. */
	static UNCell* GetOrCreatePackage(UWorld* World);

	/** Asset registry hook: clean up orphaned side-car references when a cell asset is deleted. */
	static void OnAssetRemoved(const FAssetData& AssetData);

	/** Asset registry hook: move the side-car alongside its host world when that world is renamed. */
	static void OnAssetRenamed(const FAssetData& AssetData, const FString& String);

	/** World pre-save hook: flush any in-memory cell data into the side-car before the world is written. */
	static void OnPreSaveWorldWithContext(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext);

	//~UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FNColor::NexusLightBlue; };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNCell::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	// We do not want NCells to be duplicated as they are tied to a specific world.
	virtual FAssetSupportResponse CanDuplicate(const FAssetData& InAsset) const override { return FAssetSupportResponse::NotSupported(); }
	//End UAssetDefinition

	/** Data-validation entry point invoked by UNProcGenEditorValidator for UNCell assets. */
	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);
};