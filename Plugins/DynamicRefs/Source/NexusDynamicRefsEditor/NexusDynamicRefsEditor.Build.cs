// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusDynamicRefsEditor : ModuleRules
	{
		public NexusDynamicRefsEditor(ReadOnlyTargetRules Target) : base(Target)
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
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusDynamicRefs"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusDynamicRefs", "NexusUIEditor"]);

			ShortName = "NexusDynamicRefsEditor";						
		}
	}
}