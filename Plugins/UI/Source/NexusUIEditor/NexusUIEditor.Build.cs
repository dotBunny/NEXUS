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
			
			PublicDependencyModuleNames.AddRange(["NexusUI"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor"]);

			ShortName = "NexusUIEditor";						
		}
	}
}