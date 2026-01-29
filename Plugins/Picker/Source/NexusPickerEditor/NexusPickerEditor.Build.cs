// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusPickerEditor : ModuleRules
	{
		public NexusPickerEditor(ReadOnlyTargetRules Target) : base(Target)
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
					"Engine"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusPicker"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusPicker"]);

			ShortName = "NexusPickerEditor";						
		}
	}
}