// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NActorPoolSet.h"
#include "NActorPoolSet.h"
#include "NEditorDefaults.h"
#include "Misc/DataValidation.h"

FText UAssetDefinition_NActorPoolSet::GetAssetDisplayName() const
{
	static const FText DisplayName = NSLOCTEXT("NexusActorPoolsEditor", "AssetTypeActions_NActorPoolSet", "NActorPool Set"); 
	return DisplayName;
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_NActorPoolSet::GetAssetCategories() const
{
	static const auto Categories = { FNEditorDefaults::AssetCategory };
	return Categories;
}

FText UAssetDefinition_NActorPoolSet::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UNActorPoolSet* Asset = Cast<UNActorPoolSet>(AssetData.GetAsset()))
	{
		return FText::FromString(FString::Printf(TEXT("%i Definitions"), Asset->ActorPools.Num()));
	}
	return FText::GetEmpty();
}

EDataValidationResult UAssetDefinition_NActorPoolSet::ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UNActorPoolSet* ActorPoolSet = Cast<UNActorPoolSet>(InAsset);
	if (!ActorPoolSet) return EDataValidationResult::NotValidated;

	EDataValidationResult Result = EDataValidationResult::Valid;
	for (FNActorPoolDefinition& Definition : ActorPoolSet->ActorPools)
	{
		if (Definition.ActorClass == nullptr)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("[%s] Has a null actor class reference."), *InAssetData.AssetName.ToString())));
			Result = EDataValidationResult::Invalid;
		}
	}
	return Result;
}
