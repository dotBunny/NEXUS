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
					"PlacementMode",
					"ScriptableEditorWidgets",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"BlueprintGraph",
					"Chaos",
					"ChaosCore",
					"ContentBrowser",
					"CoreUObject",
					"DeveloperSettings",
					"Engine",
					"GameplayTags",
					"GeometryCore",
					"HTTP",
					"Kismet",
					"LevelEditor",
					"Projects",
					"PropertyEditor",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UMG",
					"UMGEditor"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusCoreEditor";						
		}
	}
}