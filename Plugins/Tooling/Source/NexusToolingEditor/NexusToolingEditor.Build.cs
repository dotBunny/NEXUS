// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	// ReSharper disable once InconsistentNaming
	public class NexusToolingEditor : ModuleRules
	{
		public NexusToolingEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					// ReSharper disable once StringLiteralTypo
					"Blutility",
					"Core",
					"CommonUI",
					"UnrealEd",
					"ScriptableEditorWidgets"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"BlueprintGraph",
					"CoreUObject",
					"DataValidation",
					"DeveloperSettings",
					"Engine",
					"EditorSubsystem",
					"InputCore",
					"Json",
					"JsonUtilities",
					"Kismet",
					"LevelEditor",
					"Projects",
					"ScriptableEditorWidgets",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UnrealEd",
					"UMG",
					"UMGEditor"
				]
			);

			// NEXUS

			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusUI", "NexusUIEditor"]);

			ShortName = "NexusToolingEditor";
		}
	}
}