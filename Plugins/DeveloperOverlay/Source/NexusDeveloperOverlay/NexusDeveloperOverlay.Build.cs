// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusDeveloperOverlay : ModuleRules
	{
		public NexusDeveloperOverlay(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"UMG"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"DeveloperSettings",
					"Projects"
				]
			);
			
			// NEXUS
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusUI"]);

			ShortName = "NexusDeveloperOverlay";						
		}
	}
}