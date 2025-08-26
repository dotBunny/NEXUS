// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorCommands.h"

#include "BlueprintEditor.h"
#include "ImageUtils.h"
#include "NEditorSettings.h"
#include "NEditorUtils.h"
#include "NMetaUtils.h"
#include "Selection.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

#define LOCTEXT_NAMESPACE "NexusEditor"

void FNEditorCommands::RegisterCommands()
{
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Overwatch,
	"NCore.Help.OpenOverwatch",
	LOCTEXT("Command_Help_OpenOverwatch", "Overwatch"),
	LOCTEXT("Command_Help_Overwatch_Desc", "Opens the GitHub project's development board in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitCommunityHome"),
	EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Issues,
	"NCore.Help.OpenIssues",
	LOCTEXT("Command_Help_OpenBugReport", "Issues"),
	LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub project's issue list in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.OpenIssueTracker"),
	EUserInterfaceActionType::Button, FInputGesture());

	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_BugReport,
	"NCore.Help.OpenBugReport",
	LOCTEXT("Command_Help_OpenBugReport", "Report a Bug"),
	LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub repository's bug report form in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.ReportABug"),
	EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Roadmap,
		"NCore.Help.OpenRoadmap",
		LOCTEXT("Command_Help_OpenBugReport", "Roadmap"),
		LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub project's Roadmap in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitOnlineLearning"),
		EUserInterfaceActionType::Button, FInputGesture());	

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Documentation,
		"NCore.Help.OpenDocumentation",
		LOCTEXT("Command_Help_OpenDocumentation", "Documentation"),
		LOCTEXT("Command_Help_OpenDocumentation_Desc", "Open the documentation in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
		EUserInterfaceActionType::Button, FInputGesture());


	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Tools_LeakCheck,
	"NCore.Tools.LeakCheck",
	LOCTEXT("Command_Tools_LeakCheck", "Leak Check"),
	LOCTEXT("Command_Tools_LeakCheck_Desc", "Capture and process all UObjects over a period of 5 seconds to check for leaks."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.LeakCheck"),
	EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Tools_CaptureSelectedCamera,
	"NCore.Tools.CaptureSelectedCamera",
	LOCTEXT("Command_Tools_CaptureSelectedCamera", "Capture Selected Camera"),
	LOCTEXT("Command_Tools_CaptureSelectedCamera_Desc", "Capture the selected camera to the screenshots folder."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.CaptureSelectedCamera"),
	EUserInterfaceActionType::Button, FInputGesture());
	
	CommandList_Help = MakeShareable(new FUICommandList);
	
	CommandList_Help->MapAction(Get().CommandInfo_Help_BugReport,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpBugReport),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Overwatch,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpOverwatch),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Issues,
	FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpIssues),
	FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Roadmap,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpRoadmap),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Documentation,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpDocumentation),
		FCanExecuteAction());


	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Node_ExternalDocumentation,
			"NCore.Node.ExternalDocumentation",
			LOCTEXT("Command_Node_OpenExternalDocumentation", "External Documentation"),
			LOCTEXT("Command_Help_OpenRepository_Desc", "Open the external documentation (DocsURL) about this function."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
			EUserInterfaceActionType::Button, FInputGesture());

	CommandList_Node = MakeShareable(new FUICommandList);

	CommandList_Node->MapAction(Get().CommandInfo_Node_ExternalDocumentation,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnNodeExternalDocumentation),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::NodeExternalDocumentation_CanExecute));


	CommandList_Tools = MakeShareable(new FUICommandList);
	CommandList_Tools->MapAction(Get().CommandInfo_Tools_LeakCheck,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnToolsLeakCheck),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::ToolsLeakCheck_CanExecute));
	CommandList_Tools->MapAction(Get().CommandInfo_Tools_CaptureSelectedCamera,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnCaptureSelectedCamera),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::CaptureSelectedCamera_CanExecute));
}

void FNEditorCommands::OnHelpOverwatch()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/1"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpIssues()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/3"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpBugReport()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/dotBunny/NEXUS/issues/new/choose"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpRoadmap()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/2"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpDocumentation()
{
	FPlatformProcess::LaunchURL(TEXT("https://nexus-framework.com/docs/"),nullptr, nullptr);
}

void FNEditorCommands::OnToolsLeakCheck()
{
	UNLeakTestDelayedEditorTask::Create();
}

bool FNEditorCommands::ToolsLeakCheck_CanExecute()
{
	return true;
}

UE_DISABLE_OPTIMIZATION

void FNEditorCommands::OnCaptureSelectedCamera()
{
	const int CaptureWidth = 1920;
	const int CaptureHeight = 1080;
	
	const FString FileDirectory = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("NEXUS");
	if (!FPaths::DirectoryExists(FileDirectory))
	{
		IFileManager::Get().MakeDirectory(*FileDirectory, true);
	}
	
	for (FSelectionIterator SelectedActor(GEditor->GetSelectedActorIterator()); SelectedActor; ++SelectedActor)
	{
		if (const ACameraActor* CameraActor = Cast<ACameraActor>(*SelectedActor))
		{
			FString FileName = FString::Printf(TEXT("%s_Capture_%s.png"),
				*CameraActor->GetName(), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
			FString FilePath = FPaths::Combine(FileDirectory, *FileName);

			UWorld* World = CameraActor->GetWorld();
			
			UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(World);
			RenderTarget->bAutoGenerateMips = false;
			RenderTarget->ClearColor = FLinearColor::Black;
			RenderTarget->RenderTargetFormat = RTF_RGBA32f;
			RenderTarget->InitAutoFormat(CaptureWidth, CaptureHeight);
			RenderTarget->UpdateResourceImmediate(true);
			
			USceneCaptureComponent2D* SceneCapture = NewObject<USceneCaptureComponent2D>(World);
			SceneCapture->CaptureSource	= SCS_FinalToneCurveHDR;
			SceneCapture->CompositeMode = SCCM_Overwrite;
			SceneCapture->TextureTarget = RenderTarget;
			SceneCapture->bCaptureEveryFrame = false;
			SceneCapture->bCaptureOnMovement = false;
			SceneCapture->MaxViewDistanceOverride = -1.0f;
			SceneCapture->DetailMode = DM_Epic;
			SceneCapture->bAutoActivate = true;
			SceneCapture->bAlwaysPersistRenderingState = true;

			
			//SceneCapture->bCaptureEveryFrame = true;
			
			SceneCapture->SetWorldLocationAndRotation(CameraActor->GetActorLocation(),CameraActor->GetActorRotation());
			if (const UCameraComponent* CameraComponent = CameraActor->GetCameraComponent())
			{
				SceneCapture->FOVAngle = CameraComponent->FieldOfView;
				SceneCapture->OrthoWidth = CameraComponent->OrthoWidth;
				SceneCapture->ProjectionType = CameraComponent->ProjectionMode;
				SceneCapture->PostProcessSettings = CameraComponent->PostProcessSettings;
				SceneCapture->PostProcessBlendWeight = CameraComponent->PostProcessBlendWeight;
			}

			// Render
			SceneCapture->CaptureScene();

			FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
			TArray<FLinearColor> LinearPixels;
			RenderTargetResource->ReadLinearColorPixels(LinearPixels);

			// Scale
			TArray<FColor> Pixels;
			Pixels.Reserve(LinearPixels.Num());
			for (int i = 0; i < LinearPixels.Num(); ++i)
			{
				Pixels.Add(LinearPixels[i].ToFColor(false));
			}
			
			// Build PNG data
			TArray64<uint8> CompressedData;
			FImageUtils::PNGCompressImageArray(CaptureWidth, CaptureHeight, Pixels, CompressedData);
			FFileHelper::SaveArrayToFile(CompressedData, *FilePath);

			
			// Cleanup
			RenderTarget->ConditionalBeginDestroy();
			SceneCapture->ConditionalBeginDestroy();
			
			NE_LOG(Log, TEXT("Captured %s"), *FileName);
		}
	}
}

UE_ENABLE_OPTIMIZATION

bool FNEditorCommands::CaptureSelectedCamera_CanExecute()
{
	
	for (FSelectionIterator SelectedActor(GEditor->GetSelectedActorIterator()); SelectedActor; ++SelectedActor)
	{
		if (Cast<ACameraActor>(*SelectedActor))
		{
			return true;
		}
	}
	return false;
}

void FNEditorCommands::OnNodeExternalDocumentation()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return;
	FPlatformProcess::LaunchURL(*FNMetaUtils::GetExternalDocumentationUnsafe(Node),nullptr, nullptr);
}

bool FNEditorCommands::NodeExternalDocumentation_CanExecute()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return false;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return false;
	return FNMetaUtils::HasExternalDocumentation(Node);
}

void FNEditorCommands::BuildMenus()
{
	const FNEditorCommands Commands = Get();

	// Project Levels
	if (UToolMenu* FileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.File"))
	{
		FToolMenuSection& FileOpenSection = FileMenu->FindOrAddSection("FileOpen");
		FileOpenSection.AddSubMenu(
				"NProjectLevels",
				LOCTEXT("ProjectLevels", "Project Levels"),
				LOCTEXT("ProjectLevels_Tooltip", "A pre-defined list of levels related to the project."),
				FNewToolMenuDelegate::CreateStatic(&FillProjectLevelsSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.ProjectLevels")
			);
	}
	
	// Tools Menu
	if (UToolMenu* ToolMenus = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& ToolsSection = ToolMenus->FindOrAddSection("NEXUS");
		ToolsSection.Label = LOCTEXT("NLevelEditorTools", "NEXUS");

		ToolsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Tools_LeakCheck, Commands.CommandList_Tools);
		ToolsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Tools_CaptureSelectedCamera, Commands.CommandList_Tools);
	}
	
	// Help Menu Submenu
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help"))
	{
		FToolMenuSection& ApplicationSection = Menu->FindOrAddSection(FName("Reference"));
		ApplicationSection.AddSubMenu(
				"NEXUS",
				LOCTEXT("NHelp", "NEXUS"),
				LOCTEXT("NHelp_Tooltip", "Help related to NEXUS"),
				FNewToolMenuDelegate::CreateStatic(&FillHelpSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
			);
	}
	
	// Support for DocsURL addition to nodes
	if (UToolMenu* BlueprintNodeContextMenu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_CallFunction"))
	{
		FToolMenuSection& DocumentationSection = BlueprintNodeContextMenu->FindOrAddSection(FName("EdGraphSchemaDocumentation"));
		DocumentationSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Node_ExternalDocumentation, Commands.CommandList_Node);
	}
}

void FNEditorCommands::FillProjectLevelsSubMenu(UToolMenu* Menu)
{
	FToolMenuSection& ProjectLevelsSection = Menu->AddSection("ProjectLevels", LOCTEXT("ProjectLevels", ""));
	for (const UNEditorSettings* Settings = UNEditorSettings::Get();
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

void FNEditorCommands::FillHelpSubMenu(UToolMenu* Menu)
{
	const FNEditorCommands Commands = FNEditorCommands::Get();

	FToolMenuSection& ReferenceSection = Menu->FindOrAddSection("Reference");
	ReferenceSection.Label = LOCTEXT("NHelp_Reference", "Reference");
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Documentation,Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Overwatch, Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Roadmap, Commands.CommandList_Help);

	FToolMenuSection& SupportSection = Menu->FindOrAddSection("Support");
	SupportSection.Label = LOCTEXT("NHelp_Support", "Support");
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_BugReport, Commands.CommandList_Help);
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Issues, Commands.CommandList_Help);
}

#undef LOCTEXT_NAMESPACE
