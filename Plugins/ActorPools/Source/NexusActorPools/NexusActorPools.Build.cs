// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusActorPools : ModuleRules
	{
		public NexusActorPools(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"CoreUObject",
					"Engine"
				]
			);
			
			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"Projects"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusPicker"]);

			ShortName = "NexusActorPools";						
		}
	}
}