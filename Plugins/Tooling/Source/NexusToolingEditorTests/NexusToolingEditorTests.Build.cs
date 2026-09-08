// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	// ReSharper disable once InconsistentNaming
	public class NexusToolingEditorTests : ModuleRules
	{
		public NexusToolingEditorTests(ReadOnlyTargetRules Target) : base(Target)
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
					"CoreUObject",
					"Engine",
					"FunctionalTesting",
					"ImageCore",
					"Projects",
					"Slate",
					"SlateCore",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusToolingEditor"]);

			ShortName = "NexusToolingEditorTests";
		}
	}
}
