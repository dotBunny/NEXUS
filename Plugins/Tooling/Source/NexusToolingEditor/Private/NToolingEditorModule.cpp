// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorModule.h"

#include "NEditorUtils.h"
#include "NToolingEditorCommands.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorStyle.h"
#include "NToolingEditorUserSettings.h"
#include "NToolingEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNToolingEditorModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusTooling")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNToolingEditorModule, OnPostEngineInit);
	
	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}


void FNToolingEditorModule::ShutdownModule()
{
	FNToolingEditorCommands::RemoveMenuEntries();
	
	if (InputProcessor.IsValid())
	{
		InputProcessor.Reset();
	}
	
	FNToolingEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNToolingEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNToolingEditorStyle::Initialize();
	
	UNToolingEditorUserSettings::OnPostEngineInit();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNToolingEditorCommands::AddMenuEntries));
		
		const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();
	
		// Apply Starship style override of AppIcon
		ApplyAppIcon(Settings->ProjectAppIconPath);
	
		// Apply OS level icon to the editor's application window
		ApplyWindowIcon(Settings->ProjectWindowIconPath);
	}
	
	
}

void FNToolingEditorModule::ApplyAppIcon(const FString& IconPath)
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
			FNToolingEditorUtils::ReplaceAppIconSVG(new FSlateVectorImageBrush(FullPath, IconSize));
		}
		else
		{
			FNToolingEditorUtils::ReplaceAppIcon(new FSlateImageBrush(FullPath, IconSize));
		}
	}
	else
	{
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("Unable to find proposed project icon at %s."), *FullPath);
	}
}

void FNToolingEditorModule::ApplyWindowIcon(const FString& IconPath)
{
	// Don't have, don't do
	if (IconPath.IsEmpty())
	{
		return;
	}

	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *IconPath);
	if (FNToolingEditorUtils::ReplaceWindowIcon(BasePath))
	{
		// Register the window delegate to make sure our windows get changed, this will change the loading window as well as an indicator of success.

		WindowIconDelegateHandle = GEngine->OnPostEditorTick().AddRaw(this, &FNToolingEditorModule::ApplyWindowIconPostEditorTick);
	}
}

void FNToolingEditorModule::ApplyWindowIconPostEditorTick(float Time) const
{
	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *UNToolingEditorSettings::Get()->ProjectWindowIconPath);
	FNToolingEditorUtils::ReplaceWindowIcon(BasePath);
	GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
}

IMPLEMENT_MODULE(FNToolingEditorModule, NexusToolingEditor)


