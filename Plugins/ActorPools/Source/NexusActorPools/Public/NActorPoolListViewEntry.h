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

UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pools ListView Entry", BlueprintType, Blueprintable)
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

	UFUNCTION(BlueprintCallable)
	void Refresh() const;

protected:	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UImage> TypeImage;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> LeftText;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> CenterText;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> RightText;
	
private:
	void Reset() const;
	
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNActorPoolObject> Pool;	
};