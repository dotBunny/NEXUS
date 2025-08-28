﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditorSubsystem.h"
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
	 * Get the current editor map name.
	 */
	FORCEINLINE static FString GetCurrentMapName()
	{
		return GEditor->GetEditorWorldContext().World()->GetMapName();
	}

	/**
	 * Get the current editor map full path.
	 */
	FORCEINLINE static FString GetCurrentMapFullPath()
	{
		return GEditor->GetEditorWorldContext().World()->GetName();
	}
	/**
	 * Is in PIE mode.
	 */
	FORCEINLINE static bool IsPlayInEditor()
	{
		return GEditor->PlayWorld || GIsPlayInEditorWorld;
	}

	/**
	 * Is not in PIE mode.
	 */
	FORCEINLINE static bool IsNotPlayInEditor()
	{
		return !IsPlayInEditor();
	}

	/**
	 * Is in PIE and not paused.
	 */
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
	 * Register an <code>UDeveloperSettings</code> object with the Unreal Editor.
	* @notes Keys off SectionName
	 */
	static void RegisterSettings(UDeveloperSettings* SettingsObject);

	/**
	 * Unregister an <code>UDeveloperSettings</code> object with the Unreal Editor.
	 * @notes Keys off SectionName
	 */
	static void UnregisterSettings(const UDeveloperSettings* SettingsObject);

	/**
	 * Get the currently selected Blueprint editor.
	 * @return A pointer to the foreground Blueprint editor.
	 */
	static FBlueprintEditor* GetForegroundBlueprintEditor();

	/**
	 * Try to get the currently selected nodes in the foreground Blueprint editor.
	 */
	static bool TryGetForegroundBlueprintEditorSelectedNodes(FGraphPanelSelectionSet& OutSelection);


	/**
	 * Create a new Blueprint asset of the specified class at the given path.
	 * @param InPath The path where the Blueprint asset will be created.
	 * @param InParentClass The parent class of the Blueprint asset.
	 * @return A pointer to the newly created Blueprint asset.
	 */
	static UBlueprint* CreateBlueprint(const FString& InPath, const TSubclassOf<UObject>& InParentClass);

	
	FORCEINLINE static FEditorViewportClient* GetActiveViewportClient()
	{
		return static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	}
	
	FORCEINLINE static ULevel* GetCurrentLevel()
	{
		if (IsPlayInEditor())
		{
			return nullptr;
		}
		
		if (ULevelEditorSubsystem* LevelEditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>())
		{
			return LevelEditorSubsystem->GetCurrentLevel();
		}
		return nullptr;
	}

	
	FORCEINLINE static UWorld* GetCurrentWorld()
	{
		if (ULevel* CurrentLevel = GetCurrentLevel(); CurrentLevel != nullptr)
		{
			return CurrentLevel->OwningWorld;
		}
		return nullptr;
	}


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
	
	static void DisallowConfigFileFromStaging(const FString& Config);
	static void AllowConfigFileForStaging(const FString& Config);
	
	static void ReplaceAppIcon(FSlateImageBrush* Icon);
	static void ReplaceAppIconSVG(FSlateVectorImageBrush* Icon);
	static bool ReplaceWindowIcon(const FString& IconPath);

	static FString GetEngineBinariesPath();
};