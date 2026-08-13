// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SelectionLock/NSelectionLockMenu.h"

#include "Editor.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorStyle.h"
#include "SelectionLock/NSelectionLock.h"
#include "Selection.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "GameFramework/Actor.h"

/** The context menu this feature extends; the Outliner's menu derives from it. */
static const FName NSelectionLockActorContextMenuName = TEXT("LevelEditor.ActorContextMenu");

FName FNSelectionLockMenu::SectionName = FName("NEXUS_SelectionLock");

void FNSelectionLockMenu::AddMenuEntries()
{
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();
	if (!Settings->bSelectionLockEnabled)
	{
		return;
	}

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(NSelectionLockActorContextMenuName);
	if (Menu == nullptr)
	{
		return;
	}

	FToolMenuSection& Section = Menu->FindOrAddSection(SectionName,
		NSLOCTEXT("NexusToolingEditor", "SelectionLockSectionName", "NEXUS"));

	Section.AddMenuEntry(
		"ToggleSelectionLock",
		NSLOCTEXT("NexusToolingEditor", "ToggleSelectionLock", "Lock Selection"),
		NSLOCTEXT("NexusToolingEditor", "ToggleSelectionLock_Tooltip",
			"Lock these actors against being selected by clicking them in a level viewport. They can still be selected from the Outliner."),
		FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Status.Locked"),
		FUIAction(
			FExecuteAction::CreateStatic(&ToggleSelectionLock),
			FCanExecuteAction::CreateStatic(&ToggleSelectionLock_CanExecute),
			FIsActionChecked::CreateStatic(&ToggleSelectionLock_IsChecked)),
		EUserInterfaceActionType::Check);
}

void FNSelectionLockMenu::RemoveMenuEntries()
{
	UToolMenus* ToolMenus = UToolMenus::TryGet();
	if (ToolMenus == nullptr)
	{
		return;
	}

	UToolMenu* Menu = ToolMenus->FindMenu(NSelectionLockActorContextMenuName);
	if (Menu != nullptr)
	{
		Menu->RemoveSection(SectionName);
	}
}

TArray<AActor*> FNSelectionLockMenu::GetSelectedActors()
{
	TArray<AActor*> Actors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Actors);
	return Actors;
}

void FNSelectionLockMenu::ToggleSelectionLock()
{
	FNSelectionLock::ToggleLocked(GetSelectedActors());
}

bool FNSelectionLockMenu::ToggleSelectionLock_CanExecute()
{
	for (const AActor* Actor : GetSelectedActors())
	{
		if (FNSelectionLock::CanLock(Actor))
		{
			return true;
		}
	}

	return false;
}

bool FNSelectionLockMenu::ToggleSelectionLock_IsChecked()
{
	bool bFoundLockable = false;
	for (const AActor* Actor : GetSelectedActors())
	{
		if (!FNSelectionLock::CanLock(Actor))
		{
			continue;
		}

		if (!FNSelectionLock::IsLocked(Actor))
		{
			return false;
		}

		bFoundLockable = true;
	}

	return bFoundLockable;
}
