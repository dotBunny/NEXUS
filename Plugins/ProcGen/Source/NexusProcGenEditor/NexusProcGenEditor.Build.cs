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
					"Core",
					"DataValidation",
					"RHI",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
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
					"UnrealEd",
				]
			);
			
			
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusProcGen"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusUIEditor", "NexusProcGen"]);

			ShortName = "NexusProcGenEditor";						
		}
	}
}