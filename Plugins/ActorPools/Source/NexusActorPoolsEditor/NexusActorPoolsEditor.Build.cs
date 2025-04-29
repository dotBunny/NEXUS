// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	public class NexusActorPoolsEditor : ModuleRules
	{
		public NexusActorPoolsEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Core",
					"DataValidation", 
					"EditorSubsystem",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"CoreUObject",
					"Engine",
					"Projects",
					"SlateCore",
					"UnrealEd"
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusActorPools"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusActorPools"]);

			ShortName = "NexusActorPoolsEditor";						
		}
	}
}