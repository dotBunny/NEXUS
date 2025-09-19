// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusCoreEditor : ModuleRules
	{
		public NexusCoreEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Blutility",
					"Core",
					"DataValidation", 
					"ScriptableEditorWidgets",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"BlueprintGraph",
					"Blutility",
					"ContentBrowser",
					"CoreUObject",
					"DeveloperSettings",
					"Engine",
					"HTTP",
					"InputCore",
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
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusCoreEditor";						
		}
	}
}