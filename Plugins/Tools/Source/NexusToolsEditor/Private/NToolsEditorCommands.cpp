// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorCommands.h"

#include "BlueprintEditor.h"
#include "NEditorStyle.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NMetaUtils.h"
#include "Menus/NFixersMenu.h"
#include "Menus/NToolsMenu.h"
#include "NToolsEditorMinimal.h"
#include "NToolsEditorSettings.h"
#include "NToolsEditorStyle.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"
#include "Menus/NFixersMenuEntries.h"
#include "Menus/NToolsMenuEntries.h"

void FNToolsEditorCommands::AddMenuEntries()
{
	// Project Levels
	if (UToolMenu* FileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.File"))
	{
		FToolMenuSection& FileOpenSection = FileMenu->FindOrAddSection("FileOpen");
		FileOpenSection.AddSubMenu(
				"NEXUS_ProjectLevels",
				NSLOCTEXT("NexusCoreEditor", "ProjectLevels", "Project Levels"),
				NSLOCTEXT("NexusCoreEditor", "ProjectLevels_Tooltip", "A pre-defined list of levels related to the project."),
				FNewToolMenuDelegate::CreateStatic(&GenerateProjectLevelsSubMenu),
				false,
				FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.ProjectLevels")
			);
	}
	
	// Dynamic NEXUS Tools Menu
	if (UToolMenu* ToolMenus = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& ToolsSection = ToolMenus->FindOrAddSection("Tools");
		FToolMenuEntry Entry = FToolMenuEntry::InitSubMenu(
			"NEXUS",
			NSLOCTEXT("NexusToolsEditor", "ToolsMenu", "NEXUS"),
			NSLOCTEXT("NexusToolsEditor", "ToolsMenu_ToolTip", "An assortment of developer tools for the Unreal Editor."),
			FNewToolMenuDelegate::CreateStatic(&FNToolsMenu::GenerateMenu, false),
			false,
			FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
		);
		Entry.InsertPosition = FToolMenuInsert("FindInBlueprints", EToolMenuInsertType::After);
		ToolsSection.AddEntry(Entry);
		
		// Initialize known menu entries
		FNToolsMenuEntries::AddMenuEntries();
	}
	
	// Dynamic Find & Fix Menu (Fixers)
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NEXUS_FindAndFix",
				NSLOCTEXT("NexusToolsEditor", "FindAndFixMenu", "Find & Fix"),
				NSLOCTEXT("NexusToolsEditor", "FindAndFixMenu_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&FNFixersMenu::GenerateMenu, true),
				false,
				FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
		
		// Initialize known menu entries
		FNFixersMenuEntries::AddMenuEntries();
	}
	
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Debug"))
	{
		// Inject the Collision Visualizer into the Tools > Debug menu
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			NEXUS::ToolsEditor::CollisionVisualizer::Identifier,
			NSLOCTEXT("NexusToolsEditor", "CollisionVisualizer_Create", "Collision Visualizer"),
			NSLOCTEXT("NexusToolsEditor", "CollisionVisualizer_Create_Tooltip", "Opens the NEXUS: Collision Visualizer window."),
			FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), NEXUS::ToolsEditor::CollisionVisualizer::Icon),
			FExecuteAction::CreateStatic(&FNToolsEditorCommands::CreateCollisionVisualizerWindow));
		Entry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
		Menu->AddMenuEntry("Debug", Entry);
	}
	
	// Add in NetworkProfiler menu option if its present
	if (HasToolsProfileNetworkProfiler())
	{
		if (UToolMenu* ProfileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Profile"))
		{
			FToolMenuSection& ExternalMenu = ProfileMenu->FindOrAddSection("External");
			ExternalMenu.Label = NSLOCTEXT("NexusToolsEditor", "NLevelEditorToolsExternal", "External");
			
			ExternalMenu.AddMenuEntry(
			NEXUS::ToolsEditor::NetworkProfiler::Identifier,
			NSLOCTEXT("NexusToolsEditor", "NetworkProfiler_Open", "Network Profiler"),
			NSLOCTEXT("NexusToolsEditor", "NetworkProfiler_Open_Tooltip", "Launch external NetworkProfiler tool."),
			FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.Visualizer"),
			FExecuteAction::CreateStatic(&FNToolsEditorCommands::OpenNetworkProfiler));
		}
	}
	
	// Support for DocsURL addition to nodes
	if (UToolMenu* BlueprintNodeContextMenu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_CallFunction"))
	{
		FToolMenuSection& DocumentationSection = BlueprintNodeContextMenu->FindOrAddSection(FName("EdGraphSchemaDocumentation"));
		
		FToolMenuEntry& Entry = DocumentationSection.AddMenuEntry(
			"NEXUS_ExternalDocumentation",
			NSLOCTEXT("NexusToolsEditor", "ExternalDocumentation", "External Documentation"),
			NSLOCTEXT("NexusToolsEditor", "ExternalDocumentation_Tooltip", "Open the external documentation (DocsURL) about this function."),
			FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Icons.Documentation"),
			FExecuteAction::CreateStatic(&FNToolsEditorCommands::OnNodeExternalDocumentation));
		Entry.Visibility = FCanExecuteAction::CreateStatic(&FNToolsEditorCommands::NodeExternalDocumentation_CanExecute);
		
	}
}

void FNToolsEditorCommands::CreateCollisionVisualizerWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::ToolsEditor::CollisionVisualizer::Path, 
		NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
}

bool FNToolsEditorCommands::HasCollisionVisualizerWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
}

void FNToolsEditorCommands::OpenNetworkProfiler()
{
	const FString ExecutablePath = FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe");
	constexpr bool bLaunchDetached = true;
	constexpr bool bLaunchHidden = false;
	constexpr bool bLaunchReallyHidden = false;
	const FProcHandle ProcHandle = FPlatformProcess::CreateProc(*ExecutablePath, TEXT(""), bLaunchDetached,
		bLaunchHidden, bLaunchReallyHidden, nullptr, 0, nullptr, nullptr, nullptr);
	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogNexusToolsEditor, Error, TEXT("Unable to launch NetworkProfiler."))
	}
}

bool FNToolsEditorCommands::HasToolsProfileNetworkProfiler()
{
	return FPaths::FileExists(FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe"));
}

void FNToolsEditorCommands::GenerateProjectLevelsSubMenu(UToolMenu* Menu)
{
	FToolMenuSection& ProjectLevelsSection = Menu->AddSection("ProjectLevels", NSLOCTEXT("NexusToolsEditor", "ProjectLevels", ""));
	for (const UNToolsEditorSettings* Settings = UNToolsEditorSettings::Get();
		const FSoftObjectPath& Path : Settings->ProjectLevels)
	{
		if (!Path.IsValid())
		{
			continue;
		}
		
		const FText DisplayName = FText::FromString(Path.GetAssetName());
		const FText DisplayTooltip = FText::FromString(Path.GetAssetPathString());
		const FName AssetName = Path.GetAssetFName();

		FUIAction ButtonAction = FUIAction(
		FExecuteAction::CreateLambda([Path]()
			{
				if (const FString MapPath = Path.ToString(); MapPath.Len() > 0)
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
				}
			}),
			FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
			FIsActionChecked(),
			FIsActionButtonVisible());
		ProjectLevelsSection.AddMenuEntry(Path.GetAssetFName(), DisplayName, DisplayTooltip,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level"),
			FToolUIActionChoice(ButtonAction),
			EUserInterfaceActionType::Button, AssetName);
	}
}

void FNToolsEditorCommands::OnNodeExternalDocumentation()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return;

	// Split the data so you can have multiple URIs in the data
	TArray<FString> OutURIs;
	FNMetaUtils::GetExternalDocumentation(Node).ParseIntoArray(OutURIs, TEXT(","), true);
	for (int i = 0; i < OutURIs.Num(); i++)
	{
		FPlatformProcess::LaunchURL(*OutURIs[i].TrimStartAndEnd(),nullptr, nullptr);
	}
}

bool FNToolsEditorCommands::NodeExternalDocumentation_CanExecute()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return false;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return false;
	return FNMetaUtils::HasExternalDocumentation(Node);
}

void FNToolsEditorCommands::RemoveMenuEntries()
{
	UToolMenus* Menu = UToolMenus::Get();
	if (Menu)
	{
		// Remove Collision Visualizer
		Menu->RemoveEntry("LevelEditor.MainMenu.Tools.Debug", "Debug", 
			NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
		
		Menu->RemoveEntry("LevelEditor.MainMenu.File", "FileOpen", "NEXUS_ProjectLevels");
		
		Menu->RemoveEntry("ContentBrowser.FolderContextMenu", "PathContextBulkOperations", "NEXUS_FindAndFix");
		
		Menu->RemoveEntry("LevelEditor.MainMenu.Tools", "Tools", "NEXUS");
		
		Menu->RemoveSection("LevelEditor.MainMenu.Tools.Profile", "External");
		
		Menu->RemoveEntry("GraphEditor.GraphNodeContextMenu.K2Node_CallFunction", 
			"EdGraphSchemaDocumentation", "NCore.Node.ExternalDocumentation");
	}
}
