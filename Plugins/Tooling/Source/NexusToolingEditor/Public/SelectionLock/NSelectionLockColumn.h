// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ISceneOutlinerColumn.h"
#include "SelectionLock/NSelectionLock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/SHeaderRow.h"

class ISceneOutliner;
class UWorld;
struct FSceneOutlinerInitializationOptions;
struct ISceneOutlinerTreeItem;
template<typename ItemType> class STableRow;

/**
 * Scene Outliner column showing which actors are selection-locked, and toggling that lock on click.
 *
 * Only actor rows get a widget; folder, level and world rows, and unloaded World Partition actors,
 * render blank because there are no primitive components for a lock to act on.
 * @see <a href="https://nexus-framework.com/docs/plugins/tooling/editor-types/selection-lock/selection-lock-column/">FNSelectionLockColumn</a>
 */
class NEXUSTOOLINGEDITOR_API FNSelectionLockColumn : public ISceneOutlinerColumn
{
public:
	/**
	 * Construct the column against its owning outliner.
	 * @param Outliner The outliner this column instance belongs to.
	 */
	explicit FNSelectionLockColumn(ISceneOutliner& Outliner);

	virtual ~FNSelectionLockColumn() override = default;

	/** Register the column type and start injecting it into the level editor's Outliner. */
	static void Register();

	/** Unregister the column type and stop injecting it. */
	static void Unregister();

	/** @return the identifier this column type registers under. */
	static FName GetID() { return NEXUS::ToolingEditor::SelectionLock::ColumnIdentifier; }

	/** @return the localized label shown in the header tooltip and the column picker. */
	static FText GetDisplayName();

	//~ISceneOutlinerColumn
	virtual FName GetColumnID() override { return GetID(); }
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;
	virtual bool SupportsSorting() const override { return true; }
	virtual void SortItems(TArray<FSceneOutlinerTreeItemPtr>& OutItems, EColumnSortMode::Type SortMode) const override;
	//End ISceneOutlinerColumn

private:
	/** Handle for the actor-browser column delegate that injects this column into each Outliner. */
	static FDelegateHandle ActorBrowserColumnsHandle;

	/** Add this column to a set of Outliner initialization options as they are being assembled. */
	static void OnCreateActorBrowserColumns(FSceneOutlinerInitializationOptions& InitOptions, UWorld* World);

	/** Weak reference back to the owning outliner, used to apply a toggle across the whole selection. */
	TWeakPtr<ISceneOutliner> WeakOutliner;
};

// SNew stamps this trait from the concrete widget type, and the primary template answers false, so
// deriving from SImage is not enough to inherit its specialization. Without this every Outliner row
// reports an invalidation-unaware widget inside its invalidation panel. Mirrors what
// SceneOutlinerGutter.h declares for its own SVisibilityWidget.
template<>
struct TWidgetTypeTraits<class SNSelectionLockWidget>
{
	static constexpr bool SupportsInvalidation() { return true; }
};

/**
 * The per-row lock toggle. Draws the lock icon when the actor is locked and a muted one while the row
 * is hovered, so an unlocked column reads as empty rather than as a grid of open padlocks.
 */
class SNSelectionLockWidget : public SImage
{
public:
	SLATE_BEGIN_ARGS(SNSelectionLockWidget) {}
	SLATE_END_ARGS()

	/**
	 * Construct the toggle.
	 * @param InArgs Slate arguments.
	 * @param InWeakOutliner The owning outliner, used to resolve the current row selection.
	 * @param InWeakTreeItem The row this widget belongs to.
	 * @param InRow The owning table row, consulted for its hover state.
	 */
	void Construct(const FArguments& InArgs, TWeakPtr<ISceneOutliner> InWeakOutliner,
		TWeakPtr<ISceneOutlinerTreeItem> InWeakTreeItem, const STableRow<FSceneOutlinerTreeItemPtr>* InRow);

private:
	//~SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//End SWidget

	/** @return the normal foreground when locked, a muted tint on hover, and fully transparent otherwise. */
	FSlateColor GetLockColorAndOpacity() const;

	/** @return the actor behind this row, or nullptr when the row does not represent one. */
	AActor* GetActor() const;

	/** Toggle the whole outliner selection when this row is part of it, otherwise just this row. */
	void ToggleLock() const;

	/** The row this widget represents. */
	TWeakPtr<ISceneOutlinerTreeItem> WeakTreeItem;

	/** The outliner owning the row, used to read the current selection. */
	TWeakPtr<ISceneOutliner> WeakOutliner;

	/** The owning table row; cells are children of the row, so it outlives this widget. */
	const STableRow<FSceneOutlinerTreeItemPtr>* Row = nullptr;

	/** Cached lock brush from the module style set. */
	const FSlateBrush* LockedBrush = nullptr;
};
