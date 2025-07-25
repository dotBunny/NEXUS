// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtils.h"
#include "BlueprintEditor.h"
#include "ISettingsModule.h"
#include "KismetCompilerModule.h"
#include "NCoreEditorMinimal.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"

void FNEditorUtils::RegisterSettings(UDeveloperSettings* SettingsObject)
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		SettingsModule != nullptr)
	{
		TSharedPtr<ISettingsSection> SettingsSection = SettingsModule->RegisterSettings(
			SettingsObject->GetContainerName(),
			SettingsObject->GetCategoryName(),
			SettingsObject->GetSectionName(),
			SettingsObject->GetSectionText(), // Localized? Nope.
			SettingsObject->GetSectionDescription(), // Localized? Nope.
			SettingsObject);
	}
}

void FNEditorUtils::UnregisterSettings(const UDeveloperSettings* SettingsObject)
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings(
			SettingsObject->GetContainerName(),
			SettingsObject->GetCategoryName(),
			SettingsObject->GetSectionName());
	}
}

FBlueprintEditor* FNEditorUtils::GetForegroundBlueprintEditor()
{
	for (TArray<UObject*> EditedAssets = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllEditedAssets();
		UObject* Asset : EditedAssets)
	{
		IAssetEditorInstance* AssetEditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
		FAssetEditorToolkit* AssetEditorToolkit = static_cast<FAssetEditorToolkit*>(AssetEditorInstance);

		if (const TSharedPtr<SDockTab> Tab = AssetEditorToolkit->GetTabManager()->GetOwnerTab();
			Tab->IsForeground())
		{
  			return static_cast<FBlueprintEditor*>(AssetEditorToolkit);
		}
	}
	return nullptr;
}

bool FNEditorUtils::TryGetForegroundBlueprintEditorSelectedNodes(FGraphPanelSelectionSet& OutSelection)
{
	const FBlueprintEditor* BlueprintEditor = GetForegroundBlueprintEditor();
	if (BlueprintEditor == nullptr) return false;

	OutSelection = BlueprintEditor->GetSelectedNodes();
	return true;
}

UBlueprint* FNEditorUtils::CreateBlueprint(const FString& InPath, const TSubclassOf<UObject>& InParentClass)
{

	///We need a new object instead of the CDO
	// auto* BPFactory = NewObject<UBlueprintFactory>(GetTransientPackage());
	// BPFactory->ParentClass = InParentClass;


	
	if (StaticLoadObject(UObject::StaticClass(), nullptr, *InPath))
	{
		NE_LOG(Warning, TEXT("[FNEditorUtils::CreateBlueprint] Blueprint already exists at %s"), *InPath);
		return nullptr;
	}

	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(InParentClass))
	{
		NE_LOG(Error, TEXT("[FNEditorUtils::CreateBlueprint] Cannot create blueprint of class %s"), *InParentClass->GetName());
		return nullptr;
	}

	UPackage* Package = CreatePackage(*InPath);
	if (Package == nullptr)
	{
		NE_LOG(Error, TEXT("FNEditorUtils::CreateBlueprint] Failed to create package at %s"), *InPath);
		return nullptr;
	}

	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler")
		.GetBlueprintTypesForClass(InParentClass, BlueprintClass, BlueprintGeneratedClass);

	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(InParentClass, Package,
		*FPaths::GetBaseFilename(InPath), BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass);

	FAssetRegistryModule::AssetCreated(Blueprint);
	Blueprint->MarkPackageDirty();

	return Blueprint;
}

void FNEditorUtils::DisallowConfigFileFromStaging(const FString& Config)
{
	if (!GConfig->IsReadyForUse())
	{
		NE_LOG(Warning, TEXT("Unable to modify the DefaultGame.ini due to the GConfig not being ready."));
		return;
	}
		
	const FString RelativeConfig = FString::Printf(TEXT("%s/Config/%s.ini"), *FPaths::GetPathLeaf(FPaths::ProjectDir()), *Config);

	TArray<FString> DisallowedConfigFiles;
	GConfig->GetArray(TEXT("Staging"), TEXT("DisallowedConfigFiles"), DisallowedConfigFiles, GGameIni);
	if (!DisallowedConfigFiles.Contains(RelativeConfig))
	{
		DisallowedConfigFiles.Add(RelativeConfig);
		GConfig->SetArray(TEXT("Staging"), TEXT("DisallowedConfigFiles"), DisallowedConfigFiles, GGameIni);
		NE_LOG(Log, TEXT("[FNEditorUtils::DisallowConfigFileFromStaging] Updating DefaultGame.ini to DisallowConfig: %s"), *RelativeConfig);

		// Save config
		GConfig->Flush(false, GGameIni);
	}
}
