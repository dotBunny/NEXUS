// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INListViewEntry.h"
#include "NActorPoolObject.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "NActorPoolListViewEntry.generated.h"

class UNProcGenOperation;
class UProgressBar;
class UCommonTextBlock;

/**
 * UMG list view entry widget for displaying a single Actor Pool row (used by developer overlays).
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pools ListView Entry", BlueprintType, Blueprintable)
class NEXUSACTORPOOLS_API UNActorPoolListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}

	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:

	/** Refresh the widget's displayed fields from the current pool state. */
	UFUNCTION(BlueprintCallable)
	void Refresh() const;

protected:

	/** The list view that owns this entry. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Icon representing the pool's Actor type. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UImage> TypeImage;

	/** Progress bar visualizing how full the pool is. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	/** Left-aligned label (typically the Actor class name). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> LeftText;

	/** Center-aligned label (typically the in/out counts). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> CenterText;

	/** Right-aligned label (typically capacity or a pool descriptor). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> RightText;

private:
	/** Clear all displayed values back to their empty state. */
	void Reset() const;

	/** The pool this entry is bound to. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNActorPoolObject> Pool;
};