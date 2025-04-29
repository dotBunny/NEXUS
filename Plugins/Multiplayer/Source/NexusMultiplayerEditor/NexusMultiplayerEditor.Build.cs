// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusMultiplayerEditor : ModuleRules
	{
		public NexusMultiplayerEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"Chaos",
					"CoreUObject",
					"DeveloperSettings",
					"Engine",
					"InputCore",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UnrealEd",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor",  "NexusMultiplayer"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusMultiplayer"]);

			ShortName = "NexusMultiplayerEditor";						
		}
	}
}