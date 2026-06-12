// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NUIMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Macros/NValidationMacros.h"

#include "NProgressBarListViewEntry.generated.h"

/** Broadcast whenever the entry's label, message, or percent changes so a bound widget can refresh. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressBarListEntryChanged);

/**
 * Data model for a single row in a UNListView of progress bars. Carries a label, a status message,
 * and a 0..1 completion percent. Mutating any field broadcasts OnChanged so the bound
 * UNProgressBarListViewEntry can update in place without the owning list rebuilding its rows.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProgressBar List Entry", BlueprintType)
class NEXUSUI_API UNProgressBarListEntry : public UObject
{
	GENERATED_BODY()

public:
	/** @return The left-aligned label (e.g. the name of the thing being tracked). */
	const FString& GetLabel() const { return Label; }
	/** @return The current status message shown in the center of the row. */
	const FString& GetMessage() const { return Message; }
	/** @return The completion percent in the 0..1 range. */
	float GetPercent() const { return Percent; }

	/**
	 * Set the row's label and broadcast OnChanged if it changed.
	 * @param InLabel The new label text.
	 */
	void SetLabel(const FString& InLabel);

	/**
	 * Set the row's message and broadcast OnChanged if it changed.
	 * @param InMessage The new status message.
	 */
	void SetMessage(const FString& InMessage);

	/**
	 * Set the row's completion percent (clamped to 0..1) and broadcast OnChanged if it changed.
	 * @param InPercent The new completion percent in the 0..1 range.
	 */
	void SetPercent(float InPercent);

	/**
	 * Set the row's message and percent in one call, broadcasting OnChanged at most once.
	 * @param InMessage The new status message.
	 * @param InPercent The new completion percent in the 0..1 range.
	 */
	void SetStatus(const FString& InMessage, float InPercent);

	/** Broadcast whenever Label, Message, or Percent changes. */
	UPROPERTY(BlueprintAssignable)
	FOnProgressBarListEntryChanged OnChanged;

private:
	/** Left-aligned label for the row. */
	FString Label;
	/** Center status message for the row. */
	FString Message;
	/** Completion percent in the 0..1 range. */
	float Percent = 0.f;
};

/**
 * List-view entry widget that renders a progress bar plus left/center/right text driven by a bound
 * UNProgressBarListEntry. Binds to the data object's OnChanged delegate so message/percent updates
 * refresh the row in place; the owning list only rebuilds when entries are added or removed.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProgressBar ListView Entry", BlueprintType, Blueprintable)
class NEXUSUI_API UNProgressBarListViewEntry : public UUserWidget, public INListViewEntry
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

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

		// Will validate it here only to throw a message in log for someone to realize they haven't hooked up the widget correctly.
		N_VALIDATE(LogNexusUI, ProgressBar);
		N_VALIDATE(LogNexusUI, LeftText);
		N_VALIDATE(LogNexusUI, CenterText);
		N_VALIDATE(LogNexusUI, RightText);
	}
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

	/** Delegate callback: the bound data object's label/message/percent changed. */
	UFUNCTION()
	void OnDataChanged();

	/** The list view that owns this row. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Progress bar reflecting the bound entry's completion percent. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	/** Left-aligned label (entry label). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> LeftText;

	/** Center label (entry status message). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> CenterText;

	/** Right-aligned label (percentage). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> RightText;

private:
	/** Push the bound data object's current values into the widget tree. */
	void UpdateFromData() const;

	/** The data object backing this row; nulled when the entry is released. */
	UPROPERTY()
	TObjectPtr<UNProgressBarListEntry> Data = nullptr;
};
