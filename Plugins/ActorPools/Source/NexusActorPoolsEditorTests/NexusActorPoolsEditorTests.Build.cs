// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusActorPoolsEditorTests : ModuleRules
	{
		public NexusActorPoolsEditorTests(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"UnrealEd",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AutomationController",
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"Projects",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusActorPools", "NexusActorPoolsTests"]);

			ShortName = "NexusActorPoolsEditorTests";
		}
	}
}
