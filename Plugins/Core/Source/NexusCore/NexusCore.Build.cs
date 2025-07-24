// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusCore : ModuleRules
	{
		public NexusCore(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"CoreUObject",
					"FunctionalTesting",
					"Engine",
					"GeometryCore",
					"GeometryFramework",
					"PhysicsCore"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"ModelingComponents",
					"Projects"
				]
			);

			ShortName = "NexusCore";						
		}
	}
}