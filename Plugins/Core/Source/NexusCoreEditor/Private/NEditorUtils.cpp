// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtils.h"
#include "BlueprintEditor.h"
#include "IBlutilityModule.h"
#include "ISettingsModule.h"
#include "KismetCompilerModule.h"
#include "LevelEditor.h"
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
	
	if (StaticLoadObject(UObject::StaticClass(), nullptr, *InPath))
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to create a new UBlueprint as one already exists at the provided path(%s)."), *InPath);
		return nullptr;
	}

	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(InParentClass))
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to create a UBlueprint from UClass(%s)."), *InParentClass->GetName());
		return nullptr;
	}

	UPackage* Package = CreatePackage(*InPath);
	if (Package == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Failed to create UPackage(%s) to be used with new UBlueprint; stopping creation."), *InPath);
		return nullptr;
	}

	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler")
		.GetBlueprintTypesForClass(InParentClass, BlueprintClass, BlueprintGeneratedClass);

	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(InParentClass, Package,
		*FPaths::GetBaseFilename(InPath), BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass);

	FAssetRegistryModule::AssetCreated(Blueprint);
	
	// ReSharper disable once CppExpressionWithoutSideEffects
	Blueprint->MarkPackageDirty();

	return Blueprint;
}

void FNEditorUtils::DisallowConfigFileFromStaging(const FString& Config)
{
	const TCHAR* StagingSectionKey = TEXT("Staging");
	const TCHAR* DisallowedConfigFilesKey = TEXT("+DisallowedConfigFiles");
	const FString ProjectDefaultGamePath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sDefaultGame.ini"), *FPaths::ProjectConfigDir()));
	const FString RelativeConfig = FString::Printf(TEXT("%s/Config/%s.ini"), *FPaths::GetPathLeaf(FPaths::ProjectDir()), *Config);
	
	if (!GConfig->IsReadyForUse())
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Unable to modify the DefaultGame.ini to disallow Config(%s) from staging due to the GConfig not being ready for use."), *Config);
		return;
	}

	if (FPaths::FileExists(ProjectDefaultGamePath))
	{
		GConfig->LoadFile(ProjectDefaultGamePath);
	}
	else
	{
		GConfig->AddNewBranch(ProjectDefaultGamePath);
		UE_LOG(LogNexusCoreEditor, Verbose, TEXT("Creating missing branch(%s) in GConfig for Config(%s)"), *ProjectDefaultGamePath, *Config);
	}
	
	TArray<FString> DisallowedConfigFiles;
	FConfigFile* ProjectDefaultGameConfig = GConfig->FindConfigFile(ProjectDefaultGamePath);
	if (ProjectDefaultGameConfig == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to load project DefaultGame.ini to disallow Config(%s)."), *Config);
		return;
	}
	
	ProjectDefaultGameConfig->GetArray(StagingSectionKey, DisallowedConfigFilesKey, DisallowedConfigFiles);
	if (!DisallowedConfigFiles.Contains(RelativeConfig))
	{
		DisallowedConfigFiles.Add(RelativeConfig);
		ProjectDefaultGameConfig->SetArray(StagingSectionKey, DisallowedConfigFilesKey, DisallowedConfigFiles);
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Updating DefaultGame.ini to disallow relative Config(%s)"), *RelativeConfig);

		// Save and close the file that shouldn't be open
		GConfig->Flush(true, ProjectDefaultGamePath);
	}
}

void FNEditorUtils::AllowConfigFileForStaging(const FString& Config)
{
	const TCHAR* StagingSectionKey = TEXT("Staging");
	const TCHAR* AllowedConfigFilesKey = TEXT("+AllowedConfigFiles");
	const FString ProjectDefaultGamePath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sDefaultGame.ini"), *FPaths::ProjectConfigDir()));
	const FString RelativeConfig = FString::Printf(TEXT("%s/Config/%s.ini"), *FPaths::GetPathLeaf(FPaths::ProjectDir()), *Config);
	
	if (!GConfig->IsReadyForUse())
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Unable to modify the DefaultGame.ini to allow Config(%s) from staging due to the GConfig not being ready for use."), *Config);
		return;
	}

	if (FPaths::FileExists(ProjectDefaultGamePath))
	{
		GConfig->LoadFile(ProjectDefaultGamePath);
	}
	else
	{
		GConfig->AddNewBranch(ProjectDefaultGamePath);
		UE_LOG(LogNexusCoreEditor, Verbose, TEXT("Creating missing branch(%s) in GConfig for Config(%s)"), *ProjectDefaultGamePath, *Config);
	}
	
	TArray<FString> AllowedConfigFiles;
	FConfigFile* ProjectDefaultGameConfig = GConfig->FindConfigFile(ProjectDefaultGamePath);
	if (ProjectDefaultGameConfig == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to load project DefaultGame.ini to allow Config(%s)."), *Config);
		return;
	}
	
	ProjectDefaultGameConfig->GetArray(StagingSectionKey, AllowedConfigFilesKey, AllowedConfigFiles);
	if (!AllowedConfigFiles.Contains(RelativeConfig))
	{
		AllowedConfigFiles.Add(RelativeConfig);
		ProjectDefaultGameConfig->SetArray(StagingSectionKey, AllowedConfigFilesKey, AllowedConfigFiles);
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Updating DefaultGame.ini to allow relative Config(%s)"), *RelativeConfig);

		// Save and close the file that shouldn't be open
		GConfig->Flush(true, ProjectDefaultGamePath);
	}
}

void FNEditorUtils::SetTabClosedCallback(const FName& TabIdentifier, const SDockTab::FOnTabClosedCallback& OnTabClosedCallback)
{
	// Check Globals
	const TSharedPtr<SDockTab> GlobalActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabIdentifier);
	if (GlobalActiveTab.IsValid())
	{
		if (OnTabClosedCallback.IsBound())
		{
			GlobalActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
		}
		return;
	}
	
	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		const TSharedPtr<SDockTab> LevelEditorActiveTab = LevelEditorTabManager->FindExistingLiveTab(TabIdentifier);
		if (LevelEditorActiveTab.IsValid())
		{
			if (OnTabClosedCallback.IsBound())
			{
				LevelEditorActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
			}
		}
	}
}

void FNEditorUtils::UpdateWorkspaceItem(const FName& WidgetIdentifier, const FText& Label, const FSlateIcon& Icon)
{
	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
	
	const TArray<TSharedRef<FWorkspaceItem>>& Children = BlutilityModule->GetMenuGroup()->GetChildItems();
	for (const TSharedRef<FWorkspaceItem>& Child : Children)
	{
		if (Child->GetFName() == WidgetIdentifier)
		{
			Child->AsSpawnerEntry()->SetDisplayName(Label);
			if (Icon.IsSet())
			{
				Child->AsSpawnerEntry()->SetIcon(Icon);
			}
			break;
		}
	}
}

void FNEditorUtils::RemoveWorkspaceItem(const FName& WidgetIdentifier)
{
	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
	const TArray<TSharedRef<FWorkspaceItem>>& Children = BlutilityModule->GetMenuGroup()->GetChildItems();
	TSharedPtr<FWorkspaceItem> Found;
	for (const TSharedRef<FWorkspaceItem>& Child : Children)
	{
		if (Child->GetFName() == WidgetIdentifier)
		{
			Found = Child.ToSharedPtr();
			break;
		}
	}
	if (Found.IsValid())
	{
		BlutilityModule->GetMenuGroup()->RemoveItem(Found.ToSharedRef());
	}
}
