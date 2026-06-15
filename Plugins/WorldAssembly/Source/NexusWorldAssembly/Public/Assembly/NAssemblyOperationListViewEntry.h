// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NWorldAssemblyMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/NListView.h"
#include "Components/ProgressBar.h"
#include "Macros/NValidationMacros.h"
#include "NAssemblyOperationListViewEntry.generated.h"

class UNAssemblyOperation;
class UNProgressBarListEntry;
class UProgressBar;
class UCommonTextBlock;
struct FNStatusChannelUpdate;

/**
 * One row in the World Assembly developer overlay list — shows the operation's display name,
 * current message, and task progress.
 *
 * Binds to the operation's message/task delegates so the row stays in sync without manual polling.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Assembly Operation ListView Entry", BlueprintType, Blueprintable)
class NEXUSWORLDASSEMBLY_API UNAssemblyOperationListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

public:
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
	/** Clear all text fields and the progress bar — used between operations and on destruct. */
	void Reset() const;

	/**
	 * Apply a batch of progress-channel deltas routed from the developer overlay. Lazily creates a
	 * UNProgressBarListEntry (and child-list row) for any channel id not yet seen, then updates it.
	 * @param Changes The channels that changed since the operation's last drain.
	 */
	void ApplyChannelUpdates(const TArray<FNStatusChannelUpdate>& Changes);

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();
		
		// Will validate it here only to throw a message in log for someone to realize they haven't hooked up the widget correctly.
		N_VALIDATE(LogNexusWorldAssembly, ChildProgressListView);
		N_VALIDATE(LogNexusWorldAssembly, ProgressBar);
		N_VALIDATE(LogNexusWorldAssembly, LeftText);
		N_VALIDATE(LogNexusWorldAssembly, CenterText);
		N_VALIDATE(LogNexusWorldAssembly, RightText);
		N_VALIDATE(LogNexusWorldAssembly, CancelButton);
		
		if (IsValid(CancelButton))
		{
			CancelButton->OnClicked.AddDynamic(this, &UNAssemblyOperationListViewEntry::OnCancelButtonClicked);
		}
	}
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
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
	
	/** Left-aligned label (operation display name). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** Delegate callback: the operation's display message changed. */
	UFUNCTION()
	void OnOperationDisplayMessageChanged(const FString& NewDisplayMessage);

	/** Delegate callback: the operation's task counts changed. */
	UFUNCTION()
	void OnOperationTasksChanged(const int32 CompletedTasks, const int32 TotalTasks);

	/** Delegate callback: the entry's cancel button was clicked. */
	UFUNCTION()
	void OnCancelButtonClicked();

private:
	void UpdateCancelButtonVisibility() const;

	/** Remove every child progress bar and release its view-model — used on (re)bind and destruct. */
	void ClearChildProgressList();

	/** Operation this row is bound to; updated via NativeOnListItemObjectSet. */
	UPROPERTY()
	TObjectPtr<UNAssemblyOperation> Operation = nullptr;

	/** Child progress-bar view-models owned by this row, keyed by channel id; lazily created from deltas. */
	UPROPERTY()
	TMap<int32, TObjectPtr<UNProgressBarListEntry>> ChannelEntries;
};