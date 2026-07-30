// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusWorldAssemblyEditorTests : ModuleRules
	{
		public NexusWorldAssemblyEditorTests(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"UnrealEd",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AutomationController",
					"Chaos",
					"CoreUObject",
					"DataValidation",
					"Engine",
					"FunctionalTesting",
					"Projects",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusWorldAssembly", "NexusWorldAssemblyEditor"]);

			ShortName = "NexusWorldAssemblyEditorTests";
		}
	}
}
