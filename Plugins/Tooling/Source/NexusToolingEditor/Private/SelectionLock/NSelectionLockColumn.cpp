// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SelectionLock/NSelectionLockColumn.h"

#include "ActorTreeItem.h"
#include "ISceneOutliner.h"
#include "ISceneOutlinerMode.h"
#include "NToolingEditorStyle.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "SortHelper.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Views/STreeView.h"

/** Opacity applied to the lock icon while a row is hovered but not locked. */
static constexpr float NSelectionLockHoverOpacity = 0.35f;

/** Width of the column, matching the visibility gutter it sits beside. */
static constexpr float NSelectionLockColumnWidth = 24.0f;

/** Sort priority placing the column just after the visibility gutter (0) and unsaved marker (1). */
static constexpr uint8 NSelectionLockColumnPriority = 2;

/**
 * Resolve the actor behind an outliner row.
 * @param Item The row to resolve.
 * @return The actor, or nullptr when the row is a folder, level, world or unloaded actor.
 */
static AActor* NSelectionLockGetActorFromItem(const ISceneOutlinerTreeItem& Item)
{
	const FActorTreeItem* ActorItem = Item.CastTo<FActorTreeItem>();
	return ActorItem != nullptr ? ActorItem->Actor.Get() : nullptr;
}

FDelegateHandle FNSelectionLockColumn::ActorBrowserColumnsHandle;

FNSelectionLockColumn::FNSelectionLockColumn(ISceneOutliner& Outliner)
	: WeakOutliner(StaticCastSharedRef<ISceneOutliner>(Outliner.AsShared()))
{
}

void FNSelectionLockColumn::Register()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");
	SceneOutlinerModule.RegisterColumnType<FNSelectionLockColumn>();

	// Registering the type only makes the column available; this is what puts it in the level editor's
	// Outliner. Users can still hide it from the header dropdown, and that choice persists.
	ActorBrowserColumnsHandle = SceneOutlinerModule.OnCreateActorBrowserColumns().AddStatic(
		&FNSelectionLockColumn::OnCreateActorBrowserColumns);
}

void FNSelectionLockColumn::Unregister()
{
	FSceneOutlinerModule* SceneOutlinerModule = FModuleManager::GetModulePtr<FSceneOutlinerModule>("SceneOutliner");
	if (SceneOutlinerModule == nullptr)
	{
		return;
	}

	if (ActorBrowserColumnsHandle.IsValid())
	{
		SceneOutlinerModule->OnCreateActorBrowserColumns().Remove(ActorBrowserColumnsHandle);
		ActorBrowserColumnsHandle.Reset();
	}

	SceneOutlinerModule->UnRegisterColumnType<FNSelectionLockColumn>();
}

void FNSelectionLockColumn::OnCreateActorBrowserColumns(FSceneOutlinerInitializationOptions& InitOptions, UWorld*)
{
	InitOptions.ColumnMap.Add(GetID(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible,
		NSelectionLockColumnPriority, FCreateSceneOutlinerColumn(), true, TOptional<float>(), GetDisplayName()));
}

FText FNSelectionLockColumn::GetDisplayName()
{
	return NSLOCTEXT("NexusToolingEditor", "SelectionLockColumnName", "Selection Lock");
}

SHeaderRow::FColumn::FArguments FNSelectionLockColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		.FixedWidth(NSelectionLockColumnWidth)
		.HAlignHeader(HAlign_Left)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(NSLOCTEXT("NexusToolingEditor", "SelectionLockColumnTooltip",
			"Whether the actor can be selected by clicking it in a level viewport."))
		.HeaderContentPadding(FMargin(4.0f, 4.0f, 4.0f, 4.0f))
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FNToolingEditorStyle::Get().GetBrush("Status.Locked"))
		];
}

const TSharedRef<SWidget> FNSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	if (!FNSelectionLock::CanLock(NSelectionLockGetActorFromItem(TreeItem.Get())))
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SNSelectionLockWidget, WeakOutliner, TreeItem, &Row)
				.ToolTipText(NSLOCTEXT("NexusToolingEditor", "SelectionLockToggleTooltip",
					"Lock this actor against being selected by clicking it in a level viewport. It can still be selected here in the Outliner."))
		];
}

void FNSelectionLockColumn::SortItems(TArray<FSceneOutlinerTreeItemPtr>& OutItems, const EColumnSortMode::Type SortMode) const
{
	FSceneOutlinerSortHelper<int32, bool>()
		/** Group by row type first, so actors do not interleave with folders. */
		.Primary([this](const ISceneOutlinerTreeItem& Item)
		{
			return WeakOutliner.Pin()->GetMode()->GetTypeSortPriority(Item);
		}, SortMode)
		/** Then by lock state. */
		.Secondary([](const ISceneOutlinerTreeItem& Item)
		{
			return FNSelectionLock::IsLocked(NSelectionLockGetActorFromItem(Item));
		}, SortMode)
		.Sort(OutItems);
}

void SNSelectionLockWidget::Construct(const FArguments&, TWeakPtr<ISceneOutliner> InWeakOutliner,
	TWeakPtr<ISceneOutlinerTreeItem> InWeakTreeItem, const STableRow<FSceneOutlinerTreeItemPtr>* InRow)
{
	WeakOutliner = MoveTemp(InWeakOutliner);
	WeakTreeItem = MoveTemp(InWeakTreeItem);
	Row = InRow;
	LockedBrush = FNToolingEditorStyle::Get().GetBrush("Status.Locked");

	// The brush is always set, even when the row reads as empty: a null brush collapses the cell to
	// zero width and leaves nothing to click. The state is carried by the opacity instead.
	SImage::Construct(
		SImage::FArguments()
		.ColorAndOpacity(this, &SNSelectionLockWidget::GetLockColorAndOpacity)
		.Image(LockedBrush)
	);
}

FReply SNSelectionLockWidget::OnMouseButtonDown(const FGeometry&, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SNSelectionLockWidget::OnMouseButtonUp(const FGeometry&, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || !HasMouseCapture())
	{
		return FReply::Unhandled();
	}

	ToggleLock();
	return FReply::Handled().ReleaseMouseCapture();
}

FSlateColor SNSelectionLockWidget::GetLockColorAndOpacity() const
{
	if (FNSelectionLock::IsLocked(GetActor()))
	{
		return FSlateColor::UseForeground();
	}

	// Unlocked rows fade out entirely until hovered, so the column reads as a short list of locks
	// rather than a grid of open padlocks.
	const bool bHovered = IsHovered() || (Row != nullptr && Row->IsHovered());
	return FLinearColor(1.0f, 1.0f, 1.0f, bHovered ? NSelectionLockHoverOpacity : 0.0f);
}

AActor* SNSelectionLockWidget::GetActor() const
{
	const TSharedPtr<ISceneOutlinerTreeItem> TreeItem = WeakTreeItem.Pin();
	if (!TreeItem.IsValid())
	{
		return nullptr;
	}

	return NSelectionLockGetActorFromItem(*TreeItem);
}

void SNSelectionLockWidget::ToggleLock() const
{
	AActor* Actor = GetActor();
	if (Actor == nullptr)
	{
		return;
	}

	TArray<AActor*> Actors;

	// Clicking the toggle on a row that is part of the current selection applies to the whole selection,
	// which is how the visibility gutter beside it behaves.
	const TSharedPtr<ISceneOutliner> Outliner = WeakOutliner.Pin();
	const TSharedPtr<ISceneOutlinerTreeItem> TreeItem = WeakTreeItem.Pin();
	if (Outliner.IsValid() && TreeItem.IsValid() && Outliner->GetTree().IsItemSelected(TreeItem))
	{
		for (const FSceneOutlinerTreeItemPtr& SelectedItem : Outliner->GetTree().GetSelectedItems())
		{
			if (!SelectedItem.IsValid())
			{
				continue;
			}

			if (AActor* SelectedActor = NSelectionLockGetActorFromItem(*SelectedItem))
			{
				Actors.Add(SelectedActor);
			}
		}
	}

	if (Actors.IsEmpty())
	{
		Actors.Add(Actor);
	}

	FNSelectionLock::ToggleLocked(Actors);
}
