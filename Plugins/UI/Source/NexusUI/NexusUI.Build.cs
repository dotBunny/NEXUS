// Copyright dotBunny Inc. All Rights Reserved.
// dotBunny licenses this file to you under the BSL-1.0 license.
// See the LICENSE file in the repository root for more information.

namespace UnrealBuildTool.Rules
{
	// ReSharper disable once InconsistentNaming
	public class NexusUI : ModuleRules
	{
		public NexusUI(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"CommonUI",
					"Core",
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore",
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
			PrivateDependencyModuleNames.AddRange(["NexusCore"]);

			ShortName = "NexusUI";						
		}
	}
}