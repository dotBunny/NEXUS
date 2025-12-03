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

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#ifdef UNICODE
#define SEND_MESSAGE  SendMessageW
#else
#define SEND_MESSAGE  SendMessageA
#endif // !UNICODE
#endif

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
		NE_LOG_WARNING("[FNEditorUtils::CreateBlueprint] Blueprint already exists at %s", *InPath);
		return nullptr;
	}

	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(InParentClass))
	{
		NE_LOG_ERROR("[FNEditorUtils::CreateBlueprint] Cannot create blueprint of class %s", *InParentClass->GetName());
		return nullptr;
	}

	UPackage* Package = CreatePackage(*InPath);
	if (Package == nullptr)
	{
		NE_LOG_ERROR("FNEditorUtils::CreateBlueprint] Failed to create package at %s", *InPath);
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
		NE_LOG_WARNING("[FNEditorUtils::DisallowConfigFileFromStaging] Unable to modify the DefaultGame.ini due to the GConfig not being ready.");
		return;
	}

	if (FPaths::FileExists(ProjectDefaultGamePath))
	{
		GConfig->LoadFile(ProjectDefaultGamePath);
	}
	else
	{
		GConfig->AddNewBranch(ProjectDefaultGamePath);
		NE_LOG("[FNEditorUtils::DisallowConfigFileFromStaging] Creating branch for missing ini: %s.", *ProjectDefaultGamePath);
	}
	
	TArray<FString> DisallowedConfigFiles;
	FConfigFile* ProjectDefaultGameConfig = GConfig->FindConfigFile(ProjectDefaultGamePath);
	if (ProjectDefaultGameConfig == nullptr)
	{
		NE_LOG_ERROR("[FNEditorUtils::DisallowConfigFileFromStaging] Unable to load project DefaultGame.ini.")
		return;
	}
	
	ProjectDefaultGameConfig->GetArray(StagingSectionKey, DisallowedConfigFilesKey, DisallowedConfigFiles);
	if (!DisallowedConfigFiles.Contains(RelativeConfig))
	{
		DisallowedConfigFiles.Add(RelativeConfig);
		ProjectDefaultGameConfig->SetArray(StagingSectionKey, DisallowedConfigFilesKey, DisallowedConfigFiles);
		NE_LOG("[FNEditorUtils::DisallowConfigFileFromStaging] Updating DefaultGame.ini to DisallowConfig: %s", *ProjectDefaultGamePath);

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
		NE_LOG_WARNING("[FNEditorUtils::AllowConfigFileForStaging] Unable to modify the DefaultGame.ini due to the GConfig not being ready.");
		return;
	}

	if (FPaths::FileExists(ProjectDefaultGamePath))
	{
		GConfig->LoadFile(ProjectDefaultGamePath);
	}
	else
	{
		GConfig->AddNewBranch(ProjectDefaultGamePath);
		NE_LOG("[FNEditorUtils::AllowConfigFileForStaging] Creating branch for missing ini: %s.", *ProjectDefaultGamePath);
	}
	
	TArray<FString> AllowedConfigFiles;
	FConfigFile* ProjectDefaultGameConfig = GConfig->FindConfigFile(ProjectDefaultGamePath);
	if (ProjectDefaultGameConfig == nullptr)
	{
		NE_LOG_ERROR("[FNEditorUtils::AllowConfigFileForStaging] Unable to load project DefaultGame.ini.")
		return;
	}
	
	ProjectDefaultGameConfig->GetArray(StagingSectionKey, AllowedConfigFilesKey, AllowedConfigFiles);
	if (!AllowedConfigFiles.Contains(RelativeConfig))
	{
		AllowedConfigFiles.Add(RelativeConfig);
		ProjectDefaultGameConfig->SetArray(StagingSectionKey, AllowedConfigFilesKey, AllowedConfigFiles);
		NE_LOG("[FNEditorUtils::AllowConfigFileForStaging] Updating DefaultGame.ini to DisallowConfig: %s", *ProjectDefaultGamePath);

		// Save and close the file that shouldn't be open
		GConfig->Flush(true, ProjectDefaultGamePath);
	}
}

void FNEditorUtils::ReplaceAppIconSVG(FSlateVectorImageBrush* Icon)
{
	if (FSlateStyleSet* MutableStyleSet = const_cast<FSlateStyleSet*>(static_cast<const FSlateStyleSet*>(&FAppStyle::Get())))
	{
		MutableStyleSet->Set("AppIcon", Icon);
	}
	else
	{
		UE_LOG(LogNexusEditor, Warning, TEXT("[FNEditorUtils::ReplaceAppIconSVG] Unable to replace icon."));
	}
}

void FNEditorUtils::ReplaceAppIcon(FSlateImageBrush* Icon)
{
	if (FSlateStyleSet* MutableStyleSet = const_cast<FSlateStyleSet*>(static_cast<const FSlateStyleSet*>(&FAppStyle::Get())))
	{
		MutableStyleSet->Set("AppIcon", Icon);
	}
	else
	{
		UE_LOG(LogNexusEditor, Warning, TEXT("[FNEditorUtils::ReplaceAppIcon] Unable to replace icon."));
	}
}

bool FNEditorUtils::ReplaceWindowIcon(const FString& IconPath)
{
#if PLATFORM_WINDOWS
	const FString FinalPath = FString::Printf(TEXT("%s.ico"), *IconPath);
	// ReSharper disable CppCStyleCast, CppUE4CodingStandardNamingViolationWarning, CppZeroConstantCanBeReplacedWithNullptr
	if (FPaths::FileExists(FinalPath))
	{
		const Windows::HWND WindowHandle = FWindowsPlatformMisc::GetTopLevelWindowHandle(FWindowsPlatformProcess::GetCurrentProcessId());
		Windows::HICON hIcon = (Windows::HICON)LoadImageA(NULL, TCHAR_TO_ANSI(*FinalPath),IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE);
		
		if (hIcon)
		{
			// Set the large icon (Alt+Tab, taskbar)
			SEND_MESSAGE(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        
			// Set the small icon (window title bar)
			SEND_MESSAGE(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        
			// Also set it for the window class
			SetClassLongPtr(WindowHandle, GCLP_HICON, (LONG_PTR)hIcon);
			SetClassLongPtr(WindowHandle, GCLP_HICONSM, (LONG_PTR)hIcon);
			return true;
		}
		NE_LOG_WARNING("[FNEditorUtils::ReplaceWindowIcon] Failed to load icon from %s.", *FinalPath);
		return false;
	}
	// ReSharper restore CppCStyleCast, CppUE4CodingStandardNamingViolationWarning, CppZeroConstantCanBeReplacedWithNullptr
	NE_LOG_WARNING("[FNEditorUtils::ReplaceWindowIcon] %s Not Found.", *FinalPath);
#else
	NE_WARNING("[FNEditorUtils::ReplaceWindowIcon] Not supported on this platform.");
#endif
	return false;
}

FString FNEditorUtils::GetEngineBinariesPath()
{
	return FPaths::Combine(FPaths::EngineDir(),"Binaries");
}

void FNEditorUtils::SetTabClosedCallback(const FName& TabIdentifier, const SDockTab::FOnTabClosedCallback& OnTabClosedCallback)
{
	// Check Globals
	if (const TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabIdentifier))
	{
		if (OnTabClosedCallback.IsBound())
		{
			ActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
		}
		return;
	}
	
	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		if (const TSharedPtr<SDockTab> ActiveTab = LevelEditorTabManager->FindExistingLiveTab(TabIdentifier))
		{
			if (OnTabClosedCallback.IsBound())
			{
				ActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
			}
		}
	}
}

void FNEditorUtils::UpdateTab(const FName& TabIdentifier, const TAttribute<const FSlateBrush*>& Icon, const FText& Label, const SDockTab::FOnTabClosedCallback& OnTabClosedCallback)
{
	// Check Globals
	if (const TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabIdentifier))
	{
		if (Icon.IsSet())
		{
			ActiveTab.Get()->SetTabIcon(Icon);
		}
		
		if (!Label.IsEmpty())
		{
			ActiveTab.Get()->SetLabel(Label);
		}
		
		if (OnTabClosedCallback.IsBound())
		{
			ActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
		}
		return;
	}
	
	// Check Level Editor
	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		if (const TSharedPtr<SDockTab> ActiveTab = LevelEditorTabManager->FindExistingLiveTab(TabIdentifier))
		{
			if (Icon.IsSet())
			{
				ActiveTab.Get()->SetTabIcon(Icon);
			}
		
			if (!Label.IsEmpty())
			{
				ActiveTab.Get()->SetLabel(Label);
			}
			
			if (OnTabClosedCallback.IsBound())
			{
				ActiveTab.Get()->SetOnTabClosed(OnTabClosedCallback);
			}
			return;
		}
	}
	
	NE_LOG_WARNING("[FNEditorUtils::UpdateTab] Unable to update tab details as tab does not exist. (%s)", *TabIdentifier.ToString())
}

void FNEditorUtils::UpdateWorkspaceItem(const FName& WidgetIdentifier, const FText& Label, const FSlateIcon& Icon)
{
	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
		
	const TArray< TSharedRef<FWorkspaceItem> >& Children = BlutilityModule->GetMenuGroup()->GetChildItems();
	
	for (const TSharedRef<FWorkspaceItem>& Child : Children)
	{
		if (Child->GetFName() == WidgetIdentifier)
		{
			Child->AsSpawnerEntry()->SetDisplayName(Label);
			if (Icon.IsSet())
			{
				Child->AsSpawnerEntry()->SetIcon(Icon);
			}
		}
	}
}

void FNEditorUtils::FocusTab(const FName& TabIdentifier)
{
	// Check Globals
	if (const TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabIdentifier))
	{
		ActiveTab->ActivateInParent(SetDirectly);
		ActiveTab->FlashTab();
		return;
	}
	
	// Check Level Editor
	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		if (const TSharedPtr<SDockTab> ActiveTab = LevelEditorTabManager->FindExistingLiveTab(TabIdentifier))
		{
			ActiveTab->ActivateInParent(SetDirectly);
			ActiveTab->FlashTab();
		}
	}
}
