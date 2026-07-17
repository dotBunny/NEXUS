// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusWorldAssemblyTests : ModuleRules
	{
		public NexusWorldAssemblyTests(ReadOnlyTargetRules Target) : base(Target)
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
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"GameplayTags",
					"GeometryCore",
					"PCG",
					"Projects",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusWorldAssembly"]);

			ShortName = "NexusWorldAssemblyTests";
		}
	}
}
