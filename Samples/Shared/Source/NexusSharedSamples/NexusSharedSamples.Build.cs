// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusSharedSamples : ModuleRules
	{
		public NexusSharedSamples(ReadOnlyTargetRules Target) : base(Target)
		{
			
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"SlateCore"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"Projects",
					"UMG"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusSharedSamples";						
		}
	}
}