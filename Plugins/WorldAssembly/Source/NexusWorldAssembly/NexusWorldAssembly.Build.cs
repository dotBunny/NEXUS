// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{	
	public class NexusWorldAssembly : ModuleRules
	{
		public NexusWorldAssembly(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"GameplayTags",
					"GeometryCore",
					"PCG",
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
					"Projects",
		]
			);
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusUI"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusPicker", "NexusUI"]);

			ShortName = "NexusWorldAssembly";						
		}
	}
}