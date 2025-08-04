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
					"Engine",
					"GeometryCore",
					"GeometryFramework",
					"PhysicsCore"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"DeveloperSettings",
					"ModelingComponents",
					"Projects"
				]
			);

			ShortName = "NexusCore";						
		}
	}
}