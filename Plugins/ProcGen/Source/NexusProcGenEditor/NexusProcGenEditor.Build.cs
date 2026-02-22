// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{	
	public class NexusProcGenEditor : ModuleRules
	{
		public NexusProcGenEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Blutility",
					"Core",
					"DataValidation",
					"RHI",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"AssetTools",
					"Chaos",
					"CoreUObject",
					"EditorFramework",
					"EditorScriptingUtilities",
					"EditorSubsystem",
					"Engine",
					"InputCore",
					"LevelEditor",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UMG",
					"UMGEditor",
					"UnrealEd"
				]
			);
			
			
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCoreEditor", "NexusProcGen"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusToolingEditor", "NexusUIEditor", "NexusProcGen"]);

			ShortName = "NexusProcGenEditor";						
		}
	}
}