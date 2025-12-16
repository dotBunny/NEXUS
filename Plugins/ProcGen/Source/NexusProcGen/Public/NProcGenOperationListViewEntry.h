// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INListViewEntry.h"
#include "Blueprint/UserWidget.h"
#include "NProcGenOperationListViewEntry.generated.h"

class UNProcGenOperation;
class UProgressBar;
class UCommonTextBlock;

UCLASS(BlueprintType, Blueprintable, HideDropdown)
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
	
	void Reset() const;
	
protected:	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ChildProgressListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> LeftText;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> CenterText;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> RightText;
	
	UFUNCTION()
	void OnOperationDisplayMessageChanged(const FString& NewDisplayMessage);

	UFUNCTION()
	void OnOperationTasksChanged(const int CompletedTasks, const int TotalTasks);
	
	
private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNProcGenOperation> Operation = nullptr;	
};