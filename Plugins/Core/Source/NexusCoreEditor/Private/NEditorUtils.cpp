// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtils.h"
#include "BlueprintEditor.h"
#include "ContentBrowserModule.h"
#include "IBlutilityModule.h"
#include "IContentBrowserSingleton.h"
#include "ISettingsModule.h"
#include "KismetCompilerModule.h"
#include "LevelEditor.h"
#include "LevelEditorSubsystem.h"
#include "NCoreEditorMinimal.h"
#include "Selection.h"
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

IAssetEditorInstance* FNEditorUtils::GetForegroundAssetEditor()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem == nullptr)
	{
		return nullptr;
	}

	TArray<IAssetEditorInstance*> AssetEditorInstances = AssetEditorSubsystem->GetAllOpenEditors();
	const TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	for (IAssetEditorInstance* AssetEditorInstance : AssetEditorInstances)
	{
		// World-centric or partially initialized editors may have no tab manager or owner tab
		const TSharedPtr<FTabManager> TabManager = AssetEditorInstance->GetAssociatedTabManager();
		if (!TabManager.IsValid())
		{
			continue;
		}

		const TSharedPtr<SDockTab> Tab = TabManager->GetOwnerTab();
		if (Tab.IsValid() && Tab->IsForeground())
		{
			TSharedPtr<SWindow> ParentWindow = Tab->GetParentWindow();
			if (ParentWindow == ActiveWindow)
			{
				return AssetEditorInstance;
			}
		}
	}
	return nullptr;
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
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to create a UBlueprint from UClass(%s)."), *GetNameSafe(InParentClass));
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
	if (Blueprint == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Failed to create a UBlueprint from UClass(%s) at path(%s)."), *InParentClass->GetName(), *InPath);
		return nullptr;
	}

	FAssetRegistryModule::AssetCreated(Blueprint);

	// ReSharper disable once CppExpressionWithoutSideEffects
	Blueprint->MarkPackageDirty();

	return Blueprint;
}

ULevel* FNEditorUtils::GetCurrentLevel()
{
	if (IsPlayInEditor())
	{
		return nullptr;
	}

	ULevelEditorSubsystem* LevelEditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
	if (LevelEditorSubsystem != nullptr)
	{
		return LevelEditorSubsystem->GetCurrentLevel();
	}
	return nullptr;
}

UWorld* FNEditorUtils::GetCurrentWorld()
{
	ULevel* CurrentLevel = GetCurrentLevel();
	if (CurrentLevel != nullptr)
	{
		return CurrentLevel->OwningWorld;
	}
	return nullptr;
}

void FNEditorUtils::SelectActor(AActor* Actor)
{
	USelection* ActorSelection = GEditor->GetSelectedActors();
	ActorSelection->Modify();
	ActorSelection->DeselectAll();

	GEditor->SelectActor(Actor, true, true, true, true);
}

TArray<FString> FNEditorUtils::GetSelectedContentBrowserPaths()
{
	TArray<FString> SelectedPaths;

	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	ContentBrowser.GetSelectedFolders(SelectedPaths);

	TArray<FString> AdditionalPaths;
	ContentBrowser.GetSelectedPathViewFolders(AdditionalPaths);
	for (const FString& AdditionalPath : AdditionalPaths)
	{
		SelectedPaths.AddUnique(AdditionalPath);
	}
	return SelectedPaths;
}

// Loads the project's DefaultGame.ini, applies the add/prune in ApplyStagingConfigEntry, and flushes to
// disk when something changed. AddArrayKey selects the list to add to, RemoveArrayKey the one to prune
// ("+AllowedConfigFiles"/"+DisallowedConfigFiles"), and ActionVerb is woven into the log messages.
static void SetStagingConfigEntry(const FString& Config, const TCHAR* AddArrayKey, const TCHAR* RemoveArrayKey, const TCHAR* ActionVerb)
{
	const FString ProjectDefaultGamePath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sDefaultGame.ini"), *FPaths::ProjectConfigDir()));
	const FString RelativeConfig = FString::Printf(TEXT("%s/Config/%s.ini"), FApp::GetProjectName(), *Config);

	if (!GConfig->IsReadyForUse())
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Unable to modify the DefaultGame.ini to %s Config(%s) from staging due to the GConfig not being ready for use."), ActionVerb, *Config);
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

	FConfigFile* ProjectDefaultGameConfig = GConfig->FindConfigFile(ProjectDefaultGamePath);
	if (ProjectDefaultGameConfig == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to load project DefaultGame.ini to %s Config(%s)."), ActionVerb, *Config);
		return;
	}

	if (FNEditorUtils::ApplyStagingConfigEntry(*ProjectDefaultGameConfig, RelativeConfig, AddArrayKey, RemoveArrayKey))
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Updating DefaultGame.ini to %s relative Config(%s)"), ActionVerb, *RelativeConfig);

		// Save and close the file that shouldn't be open
		GConfig->Flush(true, ProjectDefaultGamePath);
	}
}

bool FNEditorUtils::ApplyStagingConfigEntry(FConfigFile& ConfigFile, const FString& RelativeConfig,
	const TCHAR* AddArrayKey, const TCHAR* RemoveArrayKey)
{
	const TCHAR* StagingSectionKey = TEXT("Staging");
	bool bModified = false;

	// Add to the target list.
	TArray<FString> AddConfigFiles;
	ConfigFile.GetArray(StagingSectionKey, AddArrayKey, AddConfigFiles);
	if (!AddConfigFiles.Contains(RelativeConfig))
	{
		AddConfigFiles.Add(RelativeConfig);
		ConfigFile.SetArray(StagingSectionKey, AddArrayKey, AddConfigFiles);
		bModified = true;
	}

	// Prune from the opposing list so the config never sits in both.
	TArray<FString> RemoveConfigFiles;
	ConfigFile.GetArray(StagingSectionKey, RemoveArrayKey, RemoveConfigFiles);
	if (RemoveConfigFiles.Remove(RelativeConfig) > 0)
	{
		ConfigFile.SetArray(StagingSectionKey, RemoveArrayKey, RemoveConfigFiles);
		bModified = true;
	}

	return bModified;
}

void FNEditorUtils::DisallowConfigFileFromStaging(const FString& Config)
{
	SetStagingConfigEntry(Config, TEXT("+DisallowedConfigFiles"), TEXT("+AllowedConfigFiles"), TEXT("disallow"));
}

void FNEditorUtils::AllowConfigFileForStaging(const FString& Config)
{
	SetStagingConfigEntry(Config, TEXT("+AllowedConfigFiles"), TEXT("+DisallowedConfigFiles"), TEXT("allow"));
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

void FNEditorUtils::CleanLogsFolder()
{
	TArray<FString> FilePaths;
	IFileManager& FileManager = IFileManager::Get();
	
	TArray<FString> Searches;
	Searches.Add(TEXT("*BuildOut*"));
	Searches.Add(TEXT("*-backup-*")); // Backups
	Searches.Add(TEXT("NEXUS_Compare*")); // NEXUS Compares
	Searches.Add(TEXT("NEXUS_Snapshot*")); // NEXUS Snapshots
	Searches.Add(TEXT("NEXUS_WorldAssembly*")); // NEXUS Snapshots
	Searches.Add(TEXT("*VersionSelect*")); // UE Version Selector
	Searches.Add(FString::Printf(TEXT("%s_*"), FApp::GetProjectName())); // Project secondary logs

	const FString ProjectLogDir = FPaths::ProjectLogDir();
	int32 DeleteCount = 0;

	for (const FString& Search : Searches)
	{
		FilePaths.Reset();
		FileManager.FindFilesRecursive(FilePaths, *ProjectLogDir, *Search, true, false);
		for (const FString& File : FilePaths)
		{
			FileManager.Delete(*File, false, true);
			DeleteCount++;
		}
	}
	
	if (DeleteCount > 0)
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Deleted %i files from %s."), DeleteCount, *FPaths::ProjectLogDir());
	}
	else
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("No files found to delete from %s."), *FPaths::ProjectLogDir());
	}
}

FString FNEditorUtils::GetAssetPathOnDisk(const UObject* Asset)
{
	if (!Asset)
	{
		return FString();
	}

	FString PackageName = Asset->GetOutermost()->GetName();
	FString RelativePackagePath;
	if (FPackageName::DoesPackageExist(PackageName, &RelativePackagePath))
	{
		return FPaths::ConvertRelativePathToFull(RelativePackagePath);
	}

	return FString();
}

void FNEditorUtils::UpdateWorkspaceItem(const FName& WidgetIdentifier, const FText& Label, const FSlateIcon& Icon)
{
	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
	if (BlutilityModule == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Attempting to access the Blutility Module prior to it being loaded."));
		return;
	}

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
	if (BlutilityModule == nullptr)
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Attempting to access the Blutility Module prior to it being loaded."));
		return;
	}

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
