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
					"SlateCore"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"InputCore",
					"Projects",
					"UMG"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			
			ShortName = "NexusSharedSamples";						
		}
	}
}