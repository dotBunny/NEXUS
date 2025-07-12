// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "IAssetTools.h"
#include "Cell/NCellActor.h"
#include "NEditorDefaults.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/DataAssetFactory.h"
#include "Misc/DataValidation.h"
#include "UObject/ObjectSaveContext.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TConstArrayView<FAssetCategoryPath> UAssetDefinition_NCell::GetAssetCategories() const
{
	static const auto Categories = { FNEditorDefaults::AssetCategory };
	return Categories;
}

FText UAssetDefinition_NCell::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UNCell* Asset = Cast<UNCell>(AssetData.GetAsset()))
	{
		return FText::FromString(FString::Printf(TEXT("%s"), *Asset->World.GetAssetName()));
	}
	return FText::GetEmpty();
}

FText UAssetDefinition_NCell::GetAssetDisplayName() const
{
	static const FText DisplayName = LOCTEXT("AssetTypeActions_NCell", "NCell"); 
	return DisplayName; 
}

UNCell* UAssetDefinition_NCell::GetOrCreatePackage(UWorld* World)
{
	const FString FullName = GetCellPackagePath(World->GetOutermost()->GetName());
	const FString ShortName = FPackageName::GetShortName(FullName);
	const FString PackagePath = FPaths::GetPath(FullName);

	// Load already existing object and return it
	if (FPackageName::DoesPackageExist(FullName))
	{
		return LoadObject<UNCell>(nullptr, *FullName);
	}

	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();

	// Create Asset
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UNCell* Asset = Cast<UNCell>(AssetTools.CreateAsset(ShortName, *PackagePath, UNCell::StaticClass(), Factory));

	Asset->World = TSoftObjectPtr<UWorld>(World);
	
	// Write to disk first
	UEditorAssetLibrary::SaveLoadedAsset(Asset, false);
	
	FAssetRegistryModule::AssetCreated(Asset);
	return Asset;
}

void UAssetDefinition_NCell::OnAssetRemoved(const FAssetData& AssetData)
{
	if (!AssetData.IsValid() ||
		AssetData.AssetClassPath != UWorld::StaticClass()->GetClassPathName())
	{
		return;
	}

	if (const FString SidecarPath = GetCellPackagePath(AssetData.PackageName.ToString());
		FPackageName::DoesPackageExist(SidecarPath))
	{
		UEditorAssetLibrary::DeleteAsset(SidecarPath);
	}
}

void UAssetDefinition_NCell::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
	if (!AssetData.IsValid() ||
		AssetData.AssetClassPath != UWorld::StaticClass()->GetClassPathName())
	{
		return;
	}
	
	const FString OldPath = FPackageName::GetShortName(String);
	if (const FString SidecarPath = GetCellPackagePath(OldPath);
		FPackageName::DoesPackageExist(SidecarPath))
	{
		// We need to delete the old asset as a new asset will have been created during the resave.
		UEditorAssetLibrary::DeleteAsset(SidecarPath);
	}
}

void UAssetDefinition_NCell::OnPreSaveWorldWithContext(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext)
{
	ANCellActor* CellActor = FNProcGenUtils::GetNCellActorFromWorld(World, true);

	// We don't have a cell actor, get out of here!
	if (CellActor == nullptr) return;

	// Create or get our package for the world
	FNProcGenEditorUtils::SaveNCell(World, CellActor);
}

EDataValidationResult UAssetDefinition_NCell::ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{

	const UNCell* Cell = Cast<UNCell>(InAsset);
	if (!Cell) return EDataValidationResult::NotValidated;
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (Cell->World.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("Validate_NCell_MissingWorld", "Cell {0} has no World set."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Root.Bounds.GetSize() == FVector::ZeroVector)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("Validate_NCell_BoundsSizeZero", "Cell {0} has no bounds."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Root.Hull.Vertices.Num() == 0)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("Validate_NCell_HullNoVertices", "Cell {0} hull has no vertices."), FText::FromString(Cell->GetName())));
	}
	
	if (!Cell->Root.Hull.IsConvex())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("Validate_NCell_HullNotConvex", "Cell {0} hull is not convex."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Junctions.Num() == 0)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("Validate_NCell_NoJunctions", "Cell {0} has no junctions."), FText::FromString(Cell->GetName())));
	}
	
	return Result;
}

#undef LOCTEXT_NAMESPACE
