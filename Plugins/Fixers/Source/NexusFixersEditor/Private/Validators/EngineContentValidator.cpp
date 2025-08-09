// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Validators/EngineContentValidator.h"
#include "NFixersEditorSettings.h"
#include "NFixersUtils.h"

bool UEngineContentValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (UNFixersEditorSettings::Get()->IsAssetIgnored(InAssetData.GetSoftObjectPath()))
	{
		return false;
	}
	return InObject && InAssetData.GetSoftObjectPath().ToString().StartsWith("/Engine/");
}

EDataValidationResult UEngineContentValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UNFixersEditorSettings* Settings = UNFixersEditorSettings::Get();
	EDataValidationResult Result = EDataValidationResult::Valid;
	if (Settings->EngineContentChange != NVS_Disable)
	{
		FNFixersUtils::AddResponse(Context, Settings->EngineContentChange,
			FText::FromString(TEXT("Engine content changes should be heavily scrutinized before commiting. They can be easily overwritten during upgrades or other verification processes.")));
			
		Result = EDataValidationResult::Invalid;
	}
	return Result;
}