// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NTissue.h"
#include "NEditorDefaults.h"
#include "Misc/DataValidation.h"

FText UAssetDefinition_NTissue::GetAssetDisplayName() const
{
	static const FText DisplayName = NSLOCTEXT("NexusWorldAssemblyEditor", "AssetTypeActions_NTissue", "NTissue");
	return DisplayName;
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_NTissue::GetAssetCategories() const
{
	static const auto Categories = { FNEditorDefaults::AssetCategory };
	return Categories;
}

FText UAssetDefinition_NTissue::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UNTissue* Asset = Cast<UNTissue>(AssetData.GetAsset()))
	{
		return FText::FromString(FString::Printf(TEXT("%i Cells"), Asset->Cells.Num()));
	}
	return FText::GetEmpty();
}


EDataValidationResult UAssetDefinition_NTissue::ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UNTissue* Set = Cast<UNTissue>(InAsset);
	if (!Set) return EDataValidationResult::NotValidated;

	EDataValidationResult Result = EDataValidationResult::Valid;
	for (const FNTissueEntry& Entry : Set->Cells)
	{
		if (Entry.Cell.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NTissue_MissingCell", "Tissue {0} has an entry with a missing cell."), FText::FromString(Set->GetName())));
			continue;
		}

		// A floor above the entry's own ceiling leaves no height the cell can occupy, so it would silently never
		// be placed rather than failing loudly during generation.
		if (Entry.bUseMinimumFloor && Entry.bUseMaximumCeiling && Entry.MinimumFloor > Entry.MaximumCeiling)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(
				NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NTissue_InvertedHeightWindow",
					"Tissue {0} entry {1} has a Minimum Floor ({2}) above its Maximum Ceiling ({3}), so the cell can never be placed."),
				FText::FromString(Set->GetName()),
				FText::FromString(Entry.Cell.GetAssetName()),
				FText::AsNumber(Entry.MinimumFloor),
				FText::AsNumber(Entry.MaximumCeiling)));
		}
	}
	return Result;
}