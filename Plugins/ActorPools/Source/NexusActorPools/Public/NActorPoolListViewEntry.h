// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NActorPoolObject.h"
#include "NActorPoolsMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Macros/NValidationMacros.h"
#include "NActorPoolListViewEntry.generated.h"

class UProgressBar;
class UCommonTextBlock;

/**
 * UMG list view entry widget for displaying a single Actor Pool row (used by developer overlays).
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pools ListView Entry", BlueprintType, Blueprintable)
class NEXUSACTORPOOLS_API UNActorPoolListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

public:
	
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
	/** Refresh the widget's displayed fields from the current pool state. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	void Refresh() const;

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();
		
		// Will validate it here only to throw a message in log for someone to realize they haven't hooked up the widget correctly.
		N_VALIDATE(LogNexusActorPools, TypeImage)
		N_VALIDATE(LogNexusActorPools, ProgressBar)
		N_VALIDATE(LogNexusActorPools, LeftText)
		N_VALIDATE(LogNexusActorPools, CenterText)
		N_VALIDATE(LogNexusActorPools, RightText)
	}
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

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
	UPROPERTY()
	TObjectPtr<UNActorPoolObject> Pool;
};