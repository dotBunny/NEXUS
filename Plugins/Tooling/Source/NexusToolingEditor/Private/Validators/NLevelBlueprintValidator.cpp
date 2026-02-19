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
	
	return InObject && InObject->IsA<UWorld>();
}

EDataValidationResult UNLevelBlueprintValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();

	if (Settings->ValidatorLevelBlueprint == ENValidatorSeverity::Disable)
	{
		return EDataValidationResult::NotValidated;
	}
	
	// Check Type
	const UWorld* World = Cast<UWorld>(InAsset);
	if (!World) return EDataValidationResult::NotValidated;
	
	const TArray<ULevel*> Levels = World->GetLevels();
	if (Levels.Num() <= 0) return EDataValidationResult::NotValidated;
	
	EDataValidationResult Result = EDataValidationResult::Valid;
	for (const ULevel* Level : Levels)
	{
		
		if (Level->LevelScriptBlueprint != nullptr && Level->LevelScriptBlueprint->UbergraphPages.Num() != 0)
		{
			int NodeCount = 0;
			TArray<TObjectPtr<UEdGraph>> Graphs = Level->LevelScriptBlueprint->UbergraphPages;
			for (const UEdGraph* Graph : Graphs)
			{
				for (UEdGraphNode* Node : Graph->Nodes)
				{
					if (Node->IsAutomaticallyPlacedGhostNode()) continue;
					NodeCount++;
				}
			}
			
			if (NodeCount > 0)
			{
				FNToolingEditorUtils::AddDataValidationResponse(Context, Settings->ValidatorLevelBlueprint,
				FText::FormatOrdered(TextFormat, 
					FText::FromString(FString::FromInt(NodeCount)),
					FText::FromString(Level->GetName())));
						
				Result = FNToolingEditorUtils::GetDataValidationResult(Settings->ValidatorLevelBlueprint);
			}
		}
	}
	return Result;
}
