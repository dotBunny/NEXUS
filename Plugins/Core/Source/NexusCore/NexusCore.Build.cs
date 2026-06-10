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
					"GameplayTags",
					"GeometryCore",
					"GeometryFramework",
					"PhysicsCore"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"Chaos",
					"CoreUObject",
					"DeveloperSettings",
					"GameplayTags",
					"ModelingComponents",
					"PhysicsCore",
					"Projects"
				]
			);

			ShortName = "NexusCore";						
		}
	}
}