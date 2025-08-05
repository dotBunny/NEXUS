// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreEditorModule.h"

#include "NCoreEditorMinimal.h"
#include "NEditorCommands.h"
#include "NEditorInputProcessor.h"
#include "NEditorSettings.h"
#include "NEditorStyle.h"
#include "NEditorUserSettings.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "NexusCoreEditor"

void FNCoreEditorModule::StartupModule()
{
	if (IsRunningGame()) return;
	
	if (!FNEditorUtils::IsUserControlled())
	{
		NE_LOG(Log, TEXT("[FNCoreEditorModule::StartupModule] Framework initializing in an automated environment; some functionality will be ignored."));
	}

	NE_LOG(Log, TEXT("[FNCoreEditorModule::StartupModule] Initializing ..."));
	
	UNEditorSettings::Register();
	UNEditorUserSettings::Register();

	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}

	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNCoreEditorModule, OnPostEngineInit);
}

void FNCoreEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;

	// Setup staging rules for configs
	FNEditorUtils::DisallowConfigFileFromStaging("DefaultNexusEditor");
	FNEditorUtils::AllowConfigFileForStaging("DefaultNexusGame");
	
	UNEditorUserSettings::OnPostEngineInit();
	
	FNEditorStyle::Initialize();

	if (FSlateApplication::IsInitialized())
	{
		FNEditorCommands::Register();
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNEditorCommands::BuildMenus));

		// App / Window
		const UNEditorSettings* Settings = UNEditorSettings::Get();
		const FString AppIconPath = Settings->ProjectAppIconPath;
		if (!AppIconPath.IsEmpty())
		{
			// Size set in StarshipCoreStyle
			const FVector2D IconSize(45.0f, 45.0f);
			const FString FullPath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *AppIconPath);
			if (FPaths::FileExists(FullPath))
			{
				if (FullPath.EndsWith(TEXT(".svg"), ESearchCase::IgnoreCase))
				{
					FNEditorUtils::ReplaceAppIconSVG(new FSlateVectorImageBrush(FullPath, IconSize));
				}
				else
				{
					FNEditorUtils::ReplaceAppIcon(new FSlateImageBrush(FullPath, IconSize));
				}
			}
			else
			{
				UE_LOG(LogNexusEditor, Warning, TEXT("[FNCoreEditorModule::OnPostEngineInit] Unable to find proposed project icon at %s."), *FullPath);
			}
			
			// Register the window delegate to make sure our windows get changed, this will change the loading window as well as an indicator of success.
			const FString WindowIconPath = Settings->ProjectWindowIconPath;
			if (!WindowIconPath.IsEmpty())
			{
				const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *WindowIconPath);
				if (FNEditorUtils::ReplaceWindowIcon(BasePath))
				{
					WindowIconDelegateHandle = GEngine->OnPostEditorTick().AddRaw(this, &FNCoreEditorModule::ApplyWindowIcon);
				}
			}
			
		}

		// Check for updates
		UNUpdateCheckDelayedEditorTask::Create();
	}
}

void FNCoreEditorModule::ApplyWindowIcon(float Time) const
{
	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *UNEditorSettings::Get()->ProjectWindowIconPath);
	FNEditorUtils::ReplaceWindowIcon(BasePath);
	GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)