// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{	
	public class NexusWorldAssemblyEditor : ModuleRules
	{
		public NexusWorldAssemblyEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					"Blutility",
					"Core",
					"DataValidation",
					"RHI",
					"UnrealEd"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetDefinition",
					"AssetRegistry",
					"AssetTools",
					"Chaos",
					"ContentBrowser",
					"CoreUObject",
					"DeveloperSettings",
					"EditorFramework",
					"EditorInteractiveToolsFramework",
					"EditorScriptingUtilities",
					"EditorSubsystem",
					"Engine",
					"GameplayTags",
					// Supplies FImage/FImage::ResizeTo, used to scale the badge stamped into NCell thumbnails.
					"ImageCore",
					"InputCore",
					"InteractiveToolsFramework",
					"LevelEditor",
					"PCG",
					"Projects",
					"Slate",
					"SlateCore",
					"ToolMenus",
					"UMG",
					"UMGEditor",
					// Supplies FToolkitBuilder/FToolkitSections, which build the edit mode's toolkit panel.
					"WidgetRegistration"
				]
			);
			
			
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCoreEditor", "NexusWorldAssembly"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusToolingEditor", "NexusUI", "NexusUIEditor"]);

			ShortName = "NexusWorldAssemblyEditor";						
		}
	}
}