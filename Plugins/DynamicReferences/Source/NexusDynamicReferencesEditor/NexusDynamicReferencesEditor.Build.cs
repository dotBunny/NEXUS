// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusDynamicReferencesEditor : ModuleRules
	{
		public NexusDynamicReferencesEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"CoreUObject",
					"Projects",
					"SlateCore"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusDynamicReferences"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusDynamicReferences"]);

			ShortName = "NexusDynamicReferencesEditor";						
		}
	}
}