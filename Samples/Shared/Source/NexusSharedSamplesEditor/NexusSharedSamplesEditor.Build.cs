// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusSharedSamplesEditor : ModuleRules
	{
		public NexusSharedSamplesEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",				
					"CoreUObject",
					"Engine",
					"Projects",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor",  "NexusSharedSamples"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusSharedSamples"]);

			ShortName = "NexusSharedSamplesEditor";						
		}
	}
}