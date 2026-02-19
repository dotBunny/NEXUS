// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Validators/NLevelBlueprintValidator.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorUtils.h"
#include "Engine/LevelScriptBlueprint.h"

bool UNLevelBlueprintValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                                FDataValidationContext& InContext) const
{
	if (UNToolingEditorSettings::Get()->IsAssetIgnored(InAssetData.GetSoftObjectPath()))
	{
		return false;
	}
	
	return InObject && InObject->IsA<ULevel>();
}

EDataValidationResult UNLevelBlueprintValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();

	
	
	// Check Type
	const ULevel* Level = Cast<ULevel>(InAsset);
	if (!Level) return EDataValidationResult::NotValidated;
	
	EDataValidationResult Result = EDataValidationResult::Valid;
	if (Level->LevelScriptBlueprint != nullptr && Level->LevelScriptBlueprint->UbergraphPages.Num() != 0)
	{
		Result = FNToolingEditorUtils::GetDataValidationResult(Settings->ValidatorLevelBlueprint);
	}
	return Result;
}
