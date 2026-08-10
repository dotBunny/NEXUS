// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	// ReSharper disable once InconsistentNaming
	public class NexusUIEditor : ModuleRules
	{
		public NexusUIEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					// ReSharper disable once StringLiteralTypo
					"Blutility",
					"Core",
					"CommonUI",
					// SNCommandTile's public header is built on SCompoundWidget and FToolBarStyle, so a consumer
					// including it needs SlateCore's include path of its own.
					"SlateCore",
					"UnrealEd",
					"ScriptableEditorWidgets"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"BlueprintGraph",
					"CoreUObject",
					"DeveloperSettings",
					"Engine",
					"EditorSubsystem",
					// FKey, reached through the FInputChord SNCommandTile appends to its tooltip.
					"InputCore",
					"Json",
					"JsonUtilities",
					"LevelEditor",
					"Projects",
					"Slate",
					"ToolMenus",
					"UMG",
					"UMGEditor"
				]
			);

			// NEXUS
			
			PublicDependencyModuleNames.AddRange(["NexusUI"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor"]);

			ShortName = "NexusUIEditor";						
		}
	}
}