// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusDynamicRef : ModuleRules
	{
		public NexusDynamicRef(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"Engine",
					"UMG"
				]
			);
			
			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"Projects"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore",  "NexusDeveloperOverlay"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusDynamicRef";						
		}
	}
}