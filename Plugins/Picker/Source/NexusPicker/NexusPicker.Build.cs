// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusPicker : ModuleRules
	{
		public NexusPicker(ReadOnlyTargetRules Target) : base(Target)
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
					"NavigationSystem",
					"Projects"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusPicker";						
		}
	}
}