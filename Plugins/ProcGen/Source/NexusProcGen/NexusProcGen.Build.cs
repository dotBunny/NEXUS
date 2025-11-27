// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{	
	public class NexusProcGen : ModuleRules
	{
		public NexusProcGen(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"GeometryCore",
					"UMG"
				]
			);

			PrivateDependencyModuleNames.AddRange(
		[
					"Chaos",
					"CoreUObject",
					"DeveloperSettings",					
					"Engine",
					"GeometryFramework",
					"Projects"
				]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusDeveloperOverlay", "NexusUI"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusDeveloperOverlay", "NexusUI"]);

			ShortName = "NexusProcGen";						
		}
	}
}