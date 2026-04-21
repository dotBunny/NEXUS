// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INListViewEntry.h"
#include "Blueprint/UserWidget.h"
#include "NProcGenOperationListViewEntry.generated.h"

class UNProcGenOperation;
class UProgressBar;
class UCommonTextBlock;

/**
 * One row in the ProcGen developer overlay list — shows the operation's display name,
 * current message, and task progress.
 *
 * Binds to the operation's message/task delegates so the row stays in sync without manual polling.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Operation ListView Entry", BlueprintType, Blueprintable)
class NEXUSPROCGEN_API UNProcGenOperationListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()


	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}

	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/** Clear all text fields and the progress bar — used between operations and on destruct. */
	void Reset() const;

protected:

	/** The list view that owns this row. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Nested list showing per-stage progress (optional; may be empty). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ChildProgressListView;

	/** Progress bar reflecting completed/total tasks. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	/** Left-aligned label (operation display name). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> LeftText;

	/** Center label (operation display message). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> CenterText;

	/** Right-aligned label (task counts / percentage). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> RightText;

	/** Delegate callback: the operation's display message changed. */
	UFUNCTION()
	void OnOperationDisplayMessageChanged(const FString& NewDisplayMessage);

	/** Delegate callback: the operation's task counts changed. */
	UFUNCTION()
	void OnOperationTasksChanged(const int CompletedTasks, const int TotalTasks);


private:
	/** Operation this row is bound to; updated via NativeOnListItemObjectSet. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNProcGenOperation> Operation = nullptr;
};