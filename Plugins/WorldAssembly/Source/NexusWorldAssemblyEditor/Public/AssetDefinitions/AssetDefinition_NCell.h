// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "AssetDefinitionDefault.h"
#include "NColor.h"
#include "Cell/NCell.h"
#include "AssetDefinition_NCell.generated.h"

class ANCellActor;

/**
 * Asset definition for UNCell — the on-disk side-car that mirrors a cell actor's authored bounds,
 * hull, and voxel data. Also owns the package-lifecycle hooks that keep the side-car asset in sync
 * with its host world (rename/remove/save).
 *
 * Duplication is disabled because a cell side-car is tied to a specific world and a specific cell
 * actor; copying the asset without rebinding would produce a phantom reference.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-types/asset-definitions/asset-definition-cell/">UAssetDefinition_NCell</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UAssetDefinition_NCell : public UAssetDefinitionDefault
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

	/**
	 * World pre-save hook: sync any in-memory cell data into the side-car (in-memory) before the world is written, so the
	 * recalculated actor state is captured by this level save. The side-car's own disk write is deferred to the matching
	 * post-save hook — writing a package from inside a pre-save broadcast is a re-entrant save and is unsafe.
	 */
	static void OnPreSaveWorldWithContext(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext);

	/**
	 * World post-save hook: flush the side-car package that OnPreSaveWorldWithContext dirtied for this world to disk, now
	 * that the world's own save has completed and a fresh top-level SavePackage is safe.
	 */
	static void OnPostSaveWorldWithContext(UWorld* World, FObjectPostSaveContext ObjectPostSaveContext);

	//~UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FNColor::NexusLightBlue; };
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UNCell::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;


	// We do not want NCells to be duplicated as they are tied to a specific world.
	virtual FAssetSupportResponse CanDuplicate(const FAssetData& InAsset) const override { return FAssetSupportResponse::NotSupported(); }
	//End UAssetDefinition

	/** Data-validation entry point invoked by UNWorldAssemblyEditorValidator for UNCell assets. */
	static EDataValidationResult ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context);

	/**
	 * Data-validation entry point for a cell as it currently stands in a level, rather than as it was last
	 * written to its side-car.
	 *
	 * Holds the actor to the same shape rules ValidateAsset holds the side-car to, for callers asking about
	 * a level before it is saved. Lives here rather than on the actor because the rules belong with the
	 * asset they are written for, and because this class is already a friend of ANCellActor and so can
	 * read the junctions and root details without either being opened up to everybody.
	 *
	 * @param CellActor The cell to check. A null actor, or one with no root component, yields NotValidated.
	 * @param Context Receives one error per rule the cell fails.
	 * @return Invalid where anything was found, Valid where the cell was checked and nothing was.
	 * @note Deliberately answers about live state, and so can disagree with ValidateAsset about the same
	 *       cell. The side-car is only synced from the actor in OnPreSaveWorldWithContext, so between saves
	 *       it describes an older shape. That divergence is the point: a caller asking this wants to know
	 *       what would be written, not what was.
	 */
	static EDataValidationResult ValidateCellActor(const ANCellActor* CellActor, FDataValidationContext& Context);

private:
	/**
	 * The cell shape rules, asked of whichever of the two things carries them.
	 * @param Name What to call the cell in the messages, which is the side-car's name or its level's.
	 * @param Root The cell's root details, off the side-car or off the live root component.
	 * @param JunctionCount How many junctions the cell has.
	 * @param Context Receives one error per rule failed.
	 * @return Invalid where anything was found, Valid otherwise.
	 * @note Everything the two entry points share is here so that a rule added for one is a rule added for
	 *       both. A rule that exists on the side-car path and not the live one is a rule nobody fails until
	 *       they are already on their way out the door, which is the whole thing a pre-save check exists to
	 *       avoid.
	 */
	static EDataValidationResult ValidateCellData(const FText& Name, const FNCellRootDetails& Root,
		int32 JunctionCount, FDataValidationContext& Context);
};