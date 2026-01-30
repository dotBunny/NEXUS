// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Validators/NEngineContentValidator.h"
#include "DataValidation/Public/DataValidationChangelist.h"
#include "NFixersEditorSettings.h"
#include "NFixersUtils.h"

bool UNEngineContentValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (UNFixersEditorSettings::Get()->IsAssetIgnored(InAssetData.GetSoftObjectPath()))
	{
		return false;
	}

	// Changelists are created and validated when submitting through the version control setup
	return InObject && !InObject->IsA<UDataValidationChangelist>() && InAssetData.GetSoftObjectPath().ToString().StartsWith("/Engine/");
}

EDataValidationResult UNEngineContentValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UNFixersEditorSettings* Settings = UNFixersEditorSettings::Get();
	EDataValidationResult Result = EDataValidationResult::Valid;
	if (Settings->EngineContentChange != ENValidatorSeverity::Disable)
	{
		FNFixersUtils::AddResponse(Context, Settings->EngineContentChange,
			FText::FromString(TEXT("Engine content changes should be heavily scrutinized before commiting. They can be easily overwritten during upgrades or other verification processes.\nFor more information visit https://nexus-framework.com/docs/plugins/fixers/validators/engine-content-validator/#engine-content-changed")));

		
		Result = FNFixersUtils::GetResult(Settings->EngineContentChange);
	}
	return Result;
}