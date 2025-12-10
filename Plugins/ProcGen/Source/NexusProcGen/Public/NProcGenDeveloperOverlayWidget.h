// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NProcGenDeveloperOverlayWidget.generated.h"

enum ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

UCLASS(BlueprintType, Abstract)
class UNProcGenDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void OnOperationStatusChanged(UNProcGenOperation* Operation, const ENProcGenOperationState NewState);
	
protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> OperationsList;	
	
private:
	FDelegateHandle OperationsStatusChangedDelegateHandle;
};
