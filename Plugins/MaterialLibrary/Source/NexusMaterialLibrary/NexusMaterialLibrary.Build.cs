// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusMaterialLibrary : ModuleRules
	{
		public NexusMaterialLibrary(ReadOnlyTargetRules Target) : base(Target)
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
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusMaterialLibrary";						
		}
	}
}