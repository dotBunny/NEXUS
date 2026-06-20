// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

class FConfigFile;
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
		const UWorld* World = GEditor->GetEditorWorldContext().World();
		return World != nullptr ? World->GetMapName() : TEXT("");
	}

	/** Get the current editor map full path. */
	FORCEINLINE static FString GetCurrentMapFullPath()
	{
		const UWorld* World = GEditor->GetEditorWorldContext().World();
		return World != nullptr ? World->GetName() : TEXT("");
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
		const UWorld* PlayWorld = GEditor->PlayWorld;
		return PlayWorld != nullptr && !PlayWorld->bDebugPauseExecution;
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
	 * @return The viewport client cast to FEditorViewportClient.
	 */
	FORCEINLINE static FEditorViewportClient* GetActiveViewportClient()
	{
		const FViewport* Viewport = GEditor->GetActiveViewport();
		if (Viewport != nullptr)
		{
			return static_cast<FEditorViewportClient*>(Viewport->GetClient());
		}
		return nullptr;
	}

	/**
	 * Returns the current editor level.
	 * @return The ULevel the user is editing, or nullptr while PIE is active.
	 */
	static ULevel* GetCurrentLevel();

	/**
	 * Returns the world that owns the current editor level.
	 * @return The owning UWorld, or nullptr while PIE is active or no level is loaded.
	 */
	static UWorld* GetCurrentWorld();

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
	static void SelectActor(AActor* Actor);

	/**
	 * Returns the union of folders selected in the Content Browser's main view and path view.
	 * @return A de-duplicated list of asset path roots.
	 */
	static TArray<FString> GetSelectedContentBrowserPaths();

	/** Marks Config so it will not be bundled with staged/packaged builds (undoes AllowConfigFileForStaging). */
	static void DisallowConfigFileFromStaging(const FString& Config);

	/** Marks Config so it will be bundled with staged/packaged builds (undoes DisallowConfigFileFromStaging). */
	static void AllowConfigFileForStaging(const FString& Config);

	/**
	 * Adds RelativeConfig to AddArrayKey and prunes it from RemoveArrayKey within ConfigFile's [Staging]
	 * section, so a config never lingers in both lists. Operates purely on ConfigFile — no disk I/O.
	 * @param ConfigFile The config to mutate in place.
	 * @param RelativeConfig The project-relative ini path to add (e.g. "NEXUS/Config/Foo.ini").
	 * @param AddArrayKey The Staging array to add RelativeConfig to.
	 * @param RemoveArrayKey The opposing Staging array to prune RelativeConfig from.
	 * @return true if ConfigFile was modified.
	 */
	static bool ApplyStagingConfigEntry(FConfigFile& ConfigFile, const FString& RelativeConfig,
		const TCHAR* AddArrayKey, const TCHAR* RemoveArrayKey);


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

	/**
	 * Resolves the absolute path to the package file backing Asset on disk.
	 * @param Asset Asset whose owning package to locate; may be null.
	 * @return The full, absolute path to the package file, or an empty string if Asset is null or no file exists.
	 */
	static FString GetAssetPathOnDisk(const UObject* Asset);

};