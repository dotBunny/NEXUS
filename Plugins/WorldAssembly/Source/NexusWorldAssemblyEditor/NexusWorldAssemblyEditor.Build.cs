// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{	
	public class NexusWorldAssemblyEditor : ModuleRules
	{
		public NexusWorldAssemblyEditor(ReadOnlyTargetRules Target) : base(Target)
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
					"ContentBrowser",
					"CoreUObject",
					"DeveloperSettings",
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
			PublicDependencyModuleNames.AddRange(["NexusCoreEditor", "NexusWorldAssembly"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusToolingEditor", "NexusUI", "NexusUIEditor", "NexusWorldAssembly"]);

			ShortName = "NexusWorldAssemblyEditor";						
		}
	}
}