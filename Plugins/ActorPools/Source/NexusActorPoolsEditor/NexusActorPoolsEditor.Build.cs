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
					"Blutility",
					"Core",
					"DataValidation", 
					"EditorSubsystem",
					"UnrealEd", 
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"BlueprintGraph",
					"CoreUObject",
					"DeveloperSettings",
					"EditorScriptingUtilities",
					"Engine",
					"PlacementMode",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UMG",
					"UMGEditor",
					"UnrealEd", 
				]
			);

			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCore", "NexusActorPools", "NexusUIEditor"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusActorPools", "NexusToolingEditor", "NexusUIEditor"]);

			ShortName = "NexusActorPoolsEditor";						
		}
	}
}