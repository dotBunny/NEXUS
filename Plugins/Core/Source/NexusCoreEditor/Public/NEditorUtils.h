// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditorSubsystem.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

class UAsyncEditorDelay;

/**
 * A utility methods collection for the Unreal Editor.
 */
class NEXUSCOREEDITOR_API FNEditorUtils
{
public:
	/**
	 * Indicates if the editor is in a shutdown process.
	 * @return true/false if an exit has been requested.
	 */
	FORCEINLINE static bool IsEditorShuttingDown()
	{
		return IsEngineExitRequested();
	}

	/**
	 * Are any Actors selected in the editor currently?
	 * @return true/false if any actors are selected.
	 */
	FORCEINLINE static bool HasActorsSelected()
	{
		if (GEditor->GetSelectedActorCount() == 0) return false;
		return true;
	}
	
	/** Get the current editor map name. */
	FORCEINLINE static FString GetCurrentMapName()
	{
		return GEditor->GetEditorWorldContext().World()->GetMapName();
	}

	/** Get the current editor map full path. */
	FORCEINLINE static FString GetCurrentMapFullPath()
	{
		return GEditor->GetEditorWorldContext().World()->GetName();
	}

	/** Is in PIE mode. */
	FORCEINLINE static bool IsPlayInEditor()
	{
		return GEditor->PlayWorld || GIsPlayInEditorWorld;
	}

	/** Is not in PIE mode. */
	FORCEINLINE static bool IsNotPlayInEditor()
	{
		return !IsPlayInEditor();
	}

	/** Is in PIE and not paused. */
	FORCEINLINE static bool IsPlayInEditorRunning()
	{
		return IsPlayInEditor() && !GUnrealEd->PlayWorld->bDebugPauseExecution;
	}
	
	/**
	 * Is the editor controlled by a user?
	 * @note Attempts to represent if it is safe to do things that need a fully initialized editor.
	 */
	FORCEINLINE static bool IsUserControlled()
	{
		return !IsRunningCommandlet() && !IsRunningCookCommandlet();
	}

	/**
	 * Register a UDeveloperSettings object with the Unreal Editor.
	* @notes Keys off SectionName
	 */
	static void RegisterSettings(UDeveloperSettings* SettingsObject);

	/**
	 * Unregister a UDeveloperSettings object with the Unreal Editor.
	 * @notes Keys off SectionName
	 */
	static void UnregisterSettings(const UDeveloperSettings* SettingsObject);

	/**
	 * Get the currently selected asset editor.
	 * @return A pointer to the foreground asset editor.
	 */
	static IAssetEditorInstance* GetForegroundAssetEditor();


	/**
	 * Create a new Blueprint asset of the specified class at the given path.
	 * @param InPath The path where the Blueprint asset will be created.
	 * @param InParentClass The parent class of the Blueprint asset.
	 * @return A pointer to the newly created Blueprint asset.
	 */
	static UBlueprint* CreateBlueprint(const FString& InPath, const TSubclassOf<UObject>& InParentClass);
	
	/**
	 * Returns the active editor viewport's client.
	 * @return The viewport client cast to FEditorViewportClient; does not check for null.
	 */
	FORCEINLINE static FEditorViewportClient* GetActiveViewportClient()
	{
		return static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	}

	/**
	 * Returns the current editor level.
	 * @return The ULevel the user is editing, or nullptr while PIE is active.
	 */
	FORCEINLINE static ULevel* GetCurrentLevel()
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
	
	/**
	 * Returns the world that owns the current editor level.
	 * @return The owning UWorld, or nullptr while PIE is active or no level is loaded.
	 */
	FORCEINLINE static UWorld* GetCurrentWorld()
	{
		ULevel* CurrentLevel = GetCurrentLevel();
		if (CurrentLevel != nullptr)
		{
			return CurrentLevel->OwningWorld;
		}
		return nullptr;
	}

	/**
	 * Tests whether World has never been saved (new map or in-memory only).
	 * @param World The world to inspect.
	 * @return true when World is null, has no package, or the package has never been written to disk.
	 */
	FORCEINLINE static bool IsUnsavedWorld(const UWorld* World)
	{
		if (World == nullptr) return true;
		const UPackage* Package =  World->GetPackage();
		if (Package == nullptr || Package->GetFileSize() == 0) return true;
		return !FPackageName::DoesPackageExist(Package->GetName());
	}

	/**
	 * Replaces the current actor selection with Actor.
	 * @param Actor The actor to select.
	 */
	FORCEINLINE static void SelectActor(AActor* Actor)
	{
		USelection* ActorSelection = GEditor->GetSelectedActors();
		ActorSelection->Modify();
		ActorSelection->DeselectAll();

		GEditor->SelectActor(Actor, true, true, true, true);
	}

	/**
	 * Returns the union of folders selected in the Content Browser's main view and path view.
	 * @return A de-duplicated list of asset path roots.
	 */
	static TArray<FString> GetSelectedContentBrowserPaths()
	{
		TArray<FString> SelectedPaths;

		IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
		ContentBrowser.GetSelectedFolders(SelectedPaths);

		TArray<FString> AdditionalPaths;
		ContentBrowser.GetSelectedPathViewFolders(AdditionalPaths);
		for (FString AdditionalPath : AdditionalPaths)
		{
			SelectedPaths.AddUnique(AdditionalPath);
		}
		return MoveTemp(SelectedPaths);
	}
	
	/** Marks Config so it will not be bundled with staged/packaged builds. */
	static void DisallowConfigFileFromStaging(const FString& Config);

	/** Marks Config so it will be bundled with staged/packaged builds (undoes DisallowConfigFileFromStaging). */
	static void AllowConfigFileForStaging(const FString& Config);


	/** Returns the absolute path to Engine/Binaries. */
	FORCEINLINE static FString GetEngineBinariesPath()
	{
		return FPaths::Combine(FPaths::EngineDir(), "Binaries");
	}

	/**
	 * Registers or replaces a global workspace item under WidgetIdentifier.
	 * @param WidgetIdentifier Unique name of the workspace item.
	 * @param Label Display label shown in the tab picker.
	 * @param Icon Icon shown alongside Label.
	 */
	static void UpdateWorkspaceItem(const FName& WidgetIdentifier, const FText& Label, const FSlateIcon& Icon);

	/** Unregisters the workspace item previously added under WidgetIdentifier. */
	static void RemoveWorkspaceItem(const FName& WidgetIdentifier);

	/**
	 * Registers a callback that fires when the tab with TabIdentifier is closed.
	 * @param TabIdentifier Name of the tab to hook.
	 * @param OnTabClosedCallback Delegate invoked on close.
	 */
	static void SetTabClosedCallback(const FName& TabIdentifier, const SDockTab::FOnTabClosedCallback& OnTabClosedCallback);

	/** Deletes the contents of the project's Saved/Logs folder. */
	static void CleanLogsFolder();
};