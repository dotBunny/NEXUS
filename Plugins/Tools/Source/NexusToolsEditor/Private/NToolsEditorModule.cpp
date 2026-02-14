// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorModule.h"

#include "NEditorUtils.h"
#include "NToolsEditorCommands.h"
#include "NToolsEditorMinimal.h"
#include "NToolsEditorSettings.h"
#include "NToolsEditorStyle.h"
#include "NToolsEditorUserSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNToolsEditorModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusTools")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNToolsEditorModule, OnPostEngineInit);
	
	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}


void FNToolsEditorModule::ShutdownModule()
{
	FNToolsEditorCommands::RemoveMenuEntries();
	
	if (InputProcessor.IsValid())
	{
		InputProcessor.Reset();
	}
	
	FNToolsEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNToolsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNToolsEditorStyle::Initialize();
	
	UNToolsEditorUserSettings::OnPostEngineInit();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNToolsEditorCommands::AddMenuEntries));
		
		const UNToolsEditorSettings* Settings = UNToolsEditorSettings::Get();
	
		// Apply Starship style override of AppIcon
		ApplyAppIcon(Settings->ProjectAppIconPath);
	
		// Apply OS level icon to the editor's application window
		ApplyWindowIcon(Settings->ProjectWindowIconPath);
	}
	
	
}

void FNToolsEditorModule::ApplyAppIcon(const FString& IconPath)
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
		UE_LOG(LogNexusToolsEditor, Warning, TEXT("Unable to find proposed project icon at %s."), *FullPath);
	}
}

void FNToolsEditorModule::ApplyWindowIcon(const FString& IconPath)
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

		WindowIconDelegateHandle = GEngine->OnPostEditorTick().AddRaw(this, &FNToolsEditorModule::ApplyWindowIconPostEditorTick);
	}
}

void FNToolsEditorModule::ApplyWindowIconPostEditorTick(float Time) const
{
	const FString BasePath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *UNToolsEditorSettings::Get()->ProjectWindowIconPath);
	FNEditorUtils::ReplaceWindowIcon(BasePath);
	GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
}

IMPLEMENT_MODULE(FNToolsEditorModule, NexusToolsEditor)


