// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusDeveloperConsoleEditor : ModuleRules
	{
		public NexusDeveloperConsoleEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"DataValidation", 
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"Projects",
					"UnrealEd"
				]
			);

			// NEXUS
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusDeveloperConsole"]);

			ShortName = "NexusDeveloperConsoleEditor";						
		}
	}
}