// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusActorPoolsTests : ModuleRules
	{
		public NexusActorPoolsTests(ReadOnlyTargetRules Target) : base(Target)
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
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusActorPools"]);

			ShortName = "NexusActorPoolsTests";
		}
	}
}
