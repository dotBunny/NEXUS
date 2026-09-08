// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerTab.h"

#include "NToolingEditorStyle.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/TabManager.h"
#include "Menus/NMenuEntry.h"
#include "Menus/NToolsMenu.h"
#include "TexturePacker/SNTexturePacker.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

const FName FNTexturePackerTab::TabId = TEXT("NexusTexturePacker");

TWeakPtr<SNTexturePacker> FNTexturePackerTab::ActiveWidget;

void FNTexturePackerTab::Register()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId,
			FOnSpawnTab::CreateStatic(&FNTexturePackerTab::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Texture Packer"))
		.SetTooltipText(LOCTEXT("TabTooltip",
			"Pack maps into the channels of one texture, or split a packed texture back into its maps."))
		.SetIcon(FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.TexturePacker"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	FNMenuEntry MenuEntry;
	MenuEntry.Section = TEXT("Windows");
	MenuEntry.Identifier = TabId;
	MenuEntry.DisplayName = LOCTEXT("MenuEntry", "Texture Packer");
	MenuEntry.Tooltip = LOCTEXT("MenuEntry_Tooltip",
		"Pack maps into the channels of one texture, or split a packed texture back into its maps.");
	MenuEntry.Icon = FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.TexturePacker");
	MenuEntry.Execute = FExecuteAction::CreateStatic(&FNTexturePackerTab::Open);
	FNToolsMenu::AddMenuEntry(MenuEntry);
}

void FNTexturePackerTab::Unregister()
{
	FNToolsMenu::RemoveMenuEntry(TabId);

	// Unguarded: the global tab manager is a reference rather than a pointer, and outlives this module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);

	ActiveWidget.Reset();
}

void FNTexturePackerTab::Open()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TabId);
}

void FNTexturePackerTab::OpenForUnpack(UTexture2D* Texture)
{
	Open();

	if (const TSharedPtr<SNTexturePacker> Widget = ActiveWidget.Pin())
	{
		Widget->OpenForUnpack(Texture);
	}
}

void FNTexturePackerTab::OpenForPack(const TArray<UTexture2D*>& Textures)
{
	Open();

	if (const TSharedPtr<SNTexturePacker> Widget = ActiveWidget.Pin())
	{
		Widget->OpenForPack(Textures);
	}
}

TSharedRef<SDockTab> FNTexturePackerTab::SpawnTab(const FSpawnTabArgs& Args)
{
	const TSharedRef<SNTexturePacker> Widget = SNew(SNTexturePacker);
	ActiveWidget = Widget;

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			Widget
		];
}

#undef LOCTEXT_NAMESPACE
