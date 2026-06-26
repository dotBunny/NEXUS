// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorValidator.h"

#include "NWorldAssemblyRegistry.h"
#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "AssetDefinitions/AssetDefinition_NTissue.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Misc/DataValidation.h"


class UNCell;
class UNTissue;

bool UNWorldAssemblyEditorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && (InObject->IsA<UWorld>() || InObject->IsA<UNCell>() || InObject->IsA<UNTissue>());
}

EDataValidationResult UNWorldAssemblyEditorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	if (InAsset->IsA<UWorld>())
	{
		return ValidateWorldAsset(InAssetData, InAsset, Context);
	}

	if (InAsset->IsA<UNCell>())
	{
		return UAssetDefinition_NCell::ValidateAsset(InAssetData, InAsset, Context);
	}

	if (InAsset->IsA<UNTissue>())
	{
		return UAssetDefinition_NTissue::ValidateAsset(InAssetData, InAsset, Context);
	}

	return  EDataValidationResult::NotValidated;
}

EDataValidationResult UNWorldAssemblyEditorValidator::ValidateWorldAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
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
					Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_World_MultipleNCellActors", "{0} contains multiple NCellActors."),  FText::FromName(InAssetData.AssetName)));
					Result = EDataValidationResult::Invalid;
					return Result;
				}
				CellActor = Cast<ANCellActor>(ActorIt->Get());
			}
		}
	}

	// Now that we have the cell actor, sweep its level's junctions for the cook-fatal mobility mismatch: a
	// UNCellJunctionComponent forces itself to Static mobility, so any non-Static attach parent produces a
	// Static-under-Movable error during cook. Catch it here so it surfaces in Data Validation / on save first.
	if (CellActor != nullptr)
	{
		for (const ULevel* Level : World->GetLevels())
		{
			// We don't want to check instanced levels
			if (Level->IsInstancedLevel()) continue;

			for (const UNCellJunctionComponent* Junction : FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(Level))
			{
				const USceneComponent* AttachParent = Junction->GetAttachParent();
				if (AttachParent != nullptr && AttachParent->Mobility != EComponentMobility::Static)
				{
					Result = EDataValidationResult::Invalid;
					Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_World_JunctionNonStaticParent",
						"Junction '{0}' is attached to non-Static parent '{1}'; this errors during cook. Set its owning actor/component Mobility to Static."),
						FText::FromString(Junction->GetJunctionName()),
						FText::FromString(AttachParent->GetName())));
				}
			}
		}
	}
	return Result;
}