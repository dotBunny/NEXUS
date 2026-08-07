// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/UIAction.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The organ-authoring commands of the World Assembly editor.
 *
 * Owns the declaration, binding and gating for proxy generation and level-instance management; the work lives in
 * FNWorldAssemblyEditorOrganOperations.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorOrganCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorOrganCommands& Get();

	/**
	 * Declare every organ command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void Register(const TSharedRef<FBindingContext>& Context);

	/** Every organ command, mapped to its action. Appended to the toolkit's list so the rails can resolve against it. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_AddVolume;
	TSharedPtr<FUICommandInfo> CommandInfo_GenerateProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_GenerateAllProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_ClearProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_ClearAllProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_CreateLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_CreateAllLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_UnloadLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_UnloadAllLevelInstances;

private:
	/**
	 * One command paired with the delegates it binds to.
	 */
	struct FNCommandInfoAction
	{
		TSharedPtr<FUICommandInfo> CommandInfo;
		FExecuteAction Execute;
		FCanExecuteAction CanExecute;
	};

	/** @return every organ command, paired with the delegates CommandList maps it to. */
	static TArray<FNCommandInfoAction> GetActions();
};
