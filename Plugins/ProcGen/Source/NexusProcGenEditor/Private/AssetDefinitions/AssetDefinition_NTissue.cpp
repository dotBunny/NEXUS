// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NTissue.h"
#include "NEditorDefaults.h"
#include "Misc/DataValidation.h"

FText UAssetDefinition_NTissue::GetAssetDisplayName() const
{
	static const FText DisplayName = NSLOCTEXT("NexusProcGenEditor", "AssetTypeActions_NTissue", "NTissue");
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
			Context.AddError(FText::Format(NSLOCTEXT("NexusProcGenEditor", "Validate_NTissue_MissingCell", "Tissue {0} has an entry with a missing cell."), FText::FromString(Set->GetName())));
		}
	}
	return Result;
}