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
					// Supplies FToolWidget_DragBoxPosition, persisted on UNWorldAssemblyEditorUserSettings, and
					// UE::ToolWidgets::SDraggableBoxOverlay, which floats the edit mode's rail over the viewport.
					"ToolWidgets",
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
					"UMGEditor"
				]
			);
			
			
			
			// NEXUS
			PublicDependencyModuleNames.AddRange(["NexusCoreEditor", "NexusWorldAssembly"]);
			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusToolingEditor", "NexusUI", "NexusUIEditor"]);

			ShortName = "NexusWorldAssemblyEditor";						
		}
	}
}