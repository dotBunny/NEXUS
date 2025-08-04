// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusSharedSamplesTests : ModuleRules
	{
		public NexusSharedSamplesTests(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AutomationController",
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"Projects"
					
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore",  "NexusSharedSamples"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusSharedSamples"]);

			ShortName = "NexusSharedSamplesTests";						
		}
	}
}