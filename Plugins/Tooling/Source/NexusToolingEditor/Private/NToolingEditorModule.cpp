// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorModule.h"

#include "NCoreMinimal.h"
#include "NEditorUtils.h"
#include "MultiplayerTest/NMultiplayerTestToolbarSection.h"
#include "NToolingEditorBindings.h"
#include "NToolingEditorCommands.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorStyle.h"
#include "NToolingEditorUserSettings.h"
#include "NToolingEditorUtils.h"
#include "SelectionLock/NSelectionLock.h"
#include "SelectionLock/NSelectionLockColumn.h"
#include "SelectionLock/NSelectionLockMenu.h"
#include "TexturePacker/NTexturePackerAssetActions.h"
#include "TexturePacker/NTexturePackerTab.h"
#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNToolingEditorModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusTooling")
	N_MODULE_POST_ENGINE_INIT(FNToolingEditorModule, OnPostEngineInit);

	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}


void FNToolingEditorModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT()
	// No-op in practice: the startup callback(s) are bound via CreateStatic (no `this` owner
	// for RemoveAll to match) and RegisterStartupCallback usually runs them immediately. Kept
	// for symmetry with Epic's module template; the real menu teardown is below.
	UToolMenus::UnRegisterStartupCallback(this);
	if (WindowIconDelegateHandle.IsValid() && GEngine != nullptr)
	{
		GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
	}

	FNToolingEditorCommands::RemoveMenuEntries();
	FNTexturePackerAssetActions::Unregister();
	FNTexturePackerTab::Unregister();
	FNMultiplayerTestToolbarSection::RemoveSection();
	FNSelectionLockMenu::RemoveMenuEntries();
	FNSelectionLockColumn::Unregister();
	FNSelectionLock::Shutdown();

	FNToolingEditorBindings::UnmapActions();
	FNToolingEditorBindings::Unregister();

	if (InputProcessor.IsValid())
	{
		if (FSlateApplication::IsInitialized()) // Still around
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		}
		InputProcessor.Reset();
	}

	FNToolingEditorStyle::Shutdown();
}

void FNToolingEditorModule::OnPostEngineInit()
{
	FNToolingEditorUtils::SetBlueprintEditorAssetTypes();

	if (!FNEditorUtils::IsUserControlled()) return;

	FNToolingEditorStyle::Initialize();

	UNToolingEditorUserSettings::OnPostEngineInit();

	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();

	// Read once at startup; the setting is marked ConfigRestartRequired, so it cannot change under us.
	if (Settings->bSelectionLockEnabled)
	{
		FNSelectionLock::Initialize();
		FNSelectionLockColumn::Register();
	}

	// Likewise ConfigRestartRequired: leaving the command unregistered is what keeps it out of the
	// keyboard shortcut list, and that list is built from the binding manager as it stands right now.
	if (UNToolingEditorUserSettings::Get()->bQuickHighResScreenshotEnabled)
	{
		FNToolingEditorBindings::Register();
		FNToolingEditorBindings::MapActions();
	}

	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNToolingEditorCommands::AddMenuEntries));
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNMultiplayerTestToolbarSection::AddSection));
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNSelectionLockMenu::AddMenuEntries));

		// The Content Browser's texture context menu, deferred like everything else that extends a menu.
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateStatic(FNTexturePackerAssetActions::Register));

		// Not deferred: a nomad tab spawner is registered with the tab manager rather than with a menu, and
		// the Tools menu entry goes into FNToolsMenu's registry, which is read when the menu is generated.
		FNTexturePackerTab::Register();

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
		// Brushes are heap-allocated and handed off to the engine app style set, which owns and frees them
		// (see ReplaceAppIcon[SVG]). ApplyAppIcon runs once on startup, so no repeated-swap leak occurs.
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

	WindowIconPath = FString::Printf(TEXT("%s%s"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), *IconPath);
	if (FNToolingEditorUtils::ReplaceWindowIcon(WindowIconPath))
	{
		// Register the window delegate to make sure our windows get changed, this will change the loading window as well as an indicator of success.
		WindowIconDelegateHandle = GEngine->OnPostEditorTick().AddRaw(this, &FNToolingEditorModule::ApplyWindowIconPostEditorTick);
	}
}

void FNToolingEditorModule::ApplyWindowIconPostEditorTick(float Time)
{
	GEngine->OnPostEditorTick().Remove(WindowIconDelegateHandle);
	WindowIconDelegateHandle.Reset();

	FNToolingEditorUtils::ReplaceWindowIcon(WindowIconPath);
}

IMPLEMENT_MODULE(FNToolingEditorModule, NexusToolingEditor)


