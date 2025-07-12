// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NCellSet.h"
#include "NEditorDefaults.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

FText UAssetDefinition_NCellSet::GetAssetDisplayName() const
{
	static const FText DisplayName = LOCTEXT("AssetTypeActions_NCellSet", "NCell Set"); 
	return DisplayName; 
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_NCellSet::GetAssetCategories() const
{
	static const auto Categories = { FNEditorDefaults::AssetCategory };
	return Categories;
}

FText UAssetDefinition_NCellSet::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UNCellSet* Asset = Cast<UNCellSet>(AssetData.GetAsset()))
	{
		return FText::FromString(FString::Printf(TEXT("%i Cells"), Asset->Entries.Num()));
	}
	return FText::GetEmpty();
}


EDataValidationResult UAssetDefinition_NCellSet::ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UNCellSet* Set = Cast<UNCellSet>(InAsset);
	if (!Set) return EDataValidationResult::NotValidated;

	EDataValidationResult Result = EDataValidationResult::Valid;
	for (const FNCellSetEntry& Entry : Set->Entries)
	{
		if (Entry.Cell.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("Validate_NCellSet_MissingCell", "Cell Set {0} has an entry with a missing cell."), FText::FromString(Set->GetName())));
		}
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE