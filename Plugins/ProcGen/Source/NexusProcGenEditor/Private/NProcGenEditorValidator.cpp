// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorValidator.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "AssetDefinitions/AssetDefinition_NCellSet.h"
#include "Cell/NCellActor.h"
#include "Misc/DataValidation.h"


class UNCell;
class UNCellSet;

bool UNProcGenEditorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && (InObject->IsA<UWorld>() || InObject->IsA<UNCell>() || InObject->IsA<UNCellSet>());
}

EDataValidationResult UNProcGenEditorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	if (InAsset->IsA<UWorld>())
	{
		return ValidateWorldAsset(InAssetData, InAsset, Context);
	}
	
	if (InAsset->IsA<UNCell>())
	{
		return UAssetDefinition_NCell::ValidateAsset(InAssetData, InAsset, Context);
	}

	if (InAsset->IsA<UNCellSet>())
	{
		return UAssetDefinition_NCellSet::ValidateAsset(InAssetData, InAsset, Context);
	}
	
	return  EDataValidationResult::NotValidated;
}

EDataValidationResult UNProcGenEditorValidator::ValidateWorldAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UWorld* World = Cast<UWorld>(InAsset);
	if (!World) return EDataValidationResult::NotValidated;
	
	EDataValidationResult Result = EDataValidationResult::Valid;

	ANCellActor* CellActor = nullptr;
	for ( const ULevel* Level : World->GetLevels() )
	{
		// We don't want to check instanced levels
		if (Level->IsInstancedLevel()) continue;
		for ( auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
		{
			if (ActorIt->IsA<ANCellActor>())
			{
				if (CellActor != nullptr)
				{
					Context.AddError(FText::Format(NSLOCTEXT("NexusProcGenEditor", "Validate_World_MultipleNCellActors", "{0} contains multiple NCellActors."),  FText::FromName(InAssetData.AssetName)));
					Result = EDataValidationResult::Invalid;
					return Result;
				}
				CellActor = Cast<ANCellActor>(ActorIt->Get());
			}
		}
	}
	
	// TODO: Validate now that we have our CellActor
	if(CellActor != nullptr)
	{
		
	}
	return Result;
}