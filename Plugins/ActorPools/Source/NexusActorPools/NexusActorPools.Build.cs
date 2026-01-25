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
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"UMG"
				]
			);
			
			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"DeveloperSettings",
					"Projects"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusUI"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusPicker"]);

			ShortName = "NexusActorPools";						
		}
	}
}