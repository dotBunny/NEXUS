// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusUIEditorTests : ModuleRules
	{
		public NexusUIEditorTests(ReadOnlyTargetRules Target) : base(Target)
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
					"EditorSubsystem",
					"Engine",
					"FunctionalTesting",
					"Projects",
					"Slate",
					"SlateCore",
					"UMG",
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusUI", "NexusUIEditor"]);

			ShortName = "NexusUIEditorTests";
		}
	}
}
