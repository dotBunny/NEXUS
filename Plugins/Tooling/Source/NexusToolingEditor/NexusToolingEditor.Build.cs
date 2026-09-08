// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

namespace UnrealBuildTool.Rules
{
	// ReSharper disable once InconsistentNaming
	public class NexusToolingEditor : ModuleRules
	{
		public NexusToolingEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				[
					// ReSharper disable once StringLiteralTypo
					"Blutility",
					"Core",
					"CommonUI",
					"UnrealEd",
					"ScriptableEditorWidgets"
				]
			);

			PrivateDependencyModuleNames.AddRange(
				[
					"AssetRegistry",
					"BlueprintGraph",
					"ContentBrowser",
					"ContentBrowserData",
					"CoreUObject",
					"DataValidation",
					"DeveloperSettings",
					"Engine",
					"EditorSubsystem",
					// Texture Packer: FImage and the format conversions every channel operation goes through.
					"ImageCore",
					"InputCore",
					"Json",
					"JsonUtilities",
					"Kismet",
					"LevelEditor",
					"Projects",
					"PropertyEditor",
					"SceneOutliner",
					"Slate",
					"SlateCore",
					// Texture Packer: checking a generated texture out before overwriting it.
					"SourceControl",
					"ToolMenus",
					"UMG",
					"UMGEditor",
					"WorkspaceMenuStructure",
				]
			);

			// NEXUS

			PrivateDependencyModuleNames.AddRange(["NexusCore", "NexusCoreEditor", "NexusUI", "NexusUIEditor"]);

			ShortName = "NexusToolingEditor";
		}
	}
}