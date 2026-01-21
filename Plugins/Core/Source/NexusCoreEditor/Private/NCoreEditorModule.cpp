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

void FNCoreEditorModule::StartupModule()
{
	if (IsRunningGame()) return;
	
	if (!FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Framework initializing in an automated environment; some functionality will be ignored."));
	}
	
	UNEditorSettings::Register();
	UNEditorUserSettings::Register();

	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}

	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNCoreEditorModule, OnPostEngineInit);
}

void FNCoreEditorModule::ShutdownModule()
{
	if (InputProcessor.IsValid())
	{
		InputProcessor.Reset();
	}
	IModuleInterface::ShutdownModule();
}

void FNCoreEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;

	// Setup staging rules for configs
	FNEditorUtils::DisallowConfigFileFromStaging("DefaultNexusEditor");
	FNEditorUtils::AllowConfigFileForStaging("DefaultNexusGame");
	
	UNEditorUserSettings::OnPostEngineInit();
	FNEditorStyle::Initialize();

	// Nothing works without the application being initialized
	if (!FSlateApplication::IsInitialized()) return;
	
	FNEditorCommands::Register();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNEditorCommands::BuildMenus));
	
	const UNEditorSettings* Settings = UNEditorSettings::Get();
	
	// Apply Starship style override of AppIcon
	ApplyAppIcon(Settings->ProjectAppIconPath);
	
	// Apply OS level icon to the editor's application window
	ApplyWindowIcon(Settings->ProjectWindowIconPath);
	
	// Start update check
	UNUpdateCheckDelayedEditorTask::Create();
}

void FNCoreEditorModule::ApplyAppIcon(const FString& IconPath)
{
	// Don't have, don't do
	if (IconPath.IsEmpty())
	{
		return;
	}
	
	// Size set in StarshipCoreStyle
	const FVector2D IconSize(45.0f, 45.0f);
	const FString FullPath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *IconPath);
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
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("[FNCoreEditorModule::OnPostEngineInit] Unable to find proposed project icon at %s."), *FullPath);
	}
}

void FNCoreEditorModule::ApplyWindowIcon(const FString& IconPath)
{
	// Don't have, don't do
	if (IconPath.IsEmpty())
	{
		return;
	}

	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *IconPath);
	if (FNEditorUtils::ReplaceWindowIcon(BasePath))
	{
		// Register the window delegate to make sure our windows get changed, this will change the loading window as well as an indicator of success.

		WindowIconDelegateHandle = GEngine->OnPostEditorTick().AddRaw(this, &FNCoreEditorModule::ApplyWindowIconPostEditorTick);
	}
}

void FNCoreEditorModule::ApplyWindowIconPostEditorTick(float Time) const
{
	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *UNEditorSettings::Get()->ProjectWindowIconPath);
	FNEditorUtils::ReplaceWindowIcon(BasePath);
	GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
}

IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)