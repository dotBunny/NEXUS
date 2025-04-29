// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusMultiplayer : ModuleRules
	{
		public NexusMultiplayer(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
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
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusMultiplayer";						
		}
	}
}