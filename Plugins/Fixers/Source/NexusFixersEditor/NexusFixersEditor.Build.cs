// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusFixersEditor : ModuleRules
	{
		public NexusFixersEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"Blutility",
					"BlueprintGraph",
					"CoreUObject",
					"DataValidation",
					"DeveloperSettings",
					"Engine",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UnrealEd"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor"]);

			ShortName = "NexusFixersEditor";						
		}
	}
}