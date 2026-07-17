// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusCoreTests : ModuleRules
	{
		public NexusCoreTests(ReadOnlyTargetRules Target) : base(Target)
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
					"Chaos",
					"ChaosCore",
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"GameplayTags",
					"GeometryCore",
					"Projects",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusCoreTests";
		}
	}
}
