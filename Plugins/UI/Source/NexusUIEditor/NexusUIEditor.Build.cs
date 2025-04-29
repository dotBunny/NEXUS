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
					"UnrealEd",
					"ScriptableEditorWidgets"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[			
					"BlueprintGraph",
					"CoreUObject",
					"Engine",
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
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor"]);

			ShortName = "NexusUIEditor";						
		}
	}
}