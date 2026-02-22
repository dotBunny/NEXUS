// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Validators/NEngineContentValidator.h"
#include "DataValidation/Public/DataValidationChangelist.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorUtils.h"

bool UNEngineContentValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (UNToolingEditorSettings::Get()->IsAssetIgnored(InAssetData.GetSoftObjectPath()))
	{
		return false;
	}

	// Changelists are created and validated when submitting through the version control setup
	return InObject && !InObject->IsA<UDataValidationChangelist>() && InAssetData.GetSoftObjectPath().ToString().StartsWith("/Engine/");
}

EDataValidationResult UNEngineContentValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();
	EDataValidationResult Result = EDataValidationResult::Valid;
	if (Settings->ValidatorEngineContentChange != ENValidatorSeverity::Disable)
	{
		FNToolingEditorUtils::AddDataValidationResponse(Context, Settings->ValidatorEngineContentChange,
			FText::FromString(TEXT("Engine content changes should be heavily scrutinized before commiting. They can be easily overwritten during upgrades or other verification processes.\nFor more information visit https://nexus-framework.com/docs/plugins/tooling/validators/engine-content-validator/#engine-content-changed")));

		
		Result = FNToolingEditorUtils::GetDataValidationResult(Settings->ValidatorEngineContentChange);
	}
	return Result;
}