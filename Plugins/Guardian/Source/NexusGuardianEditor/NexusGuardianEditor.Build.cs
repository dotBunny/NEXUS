// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusGuardianEditor : ModuleRules
	{
		public NexusGuardianEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"EditorSubsystem",
					"Engine",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UnrealEd"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusGuardian"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusGuardian", "NexusToolingEditor", "NexusUIEditor"]);

			ShortName = "NexusGuardianEditor";						
		}
	}
}