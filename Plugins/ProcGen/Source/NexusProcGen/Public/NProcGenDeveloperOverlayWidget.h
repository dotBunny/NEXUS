// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NProcGenDeveloperOverlayWidget.generated.h"

enum class ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "ProcGen Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSPROCGEN_API UNProcGenDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void OnOperationStatusChanged(UNProcGenOperation* Operation, const ENProcGenOperationState NewState);
	
protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> OperationsList;	
	
private:
	void UpdateBanner() const;
	
	FDelegateHandle OperationsStatusChangedDelegateHandle;
	static bool ShouldShowOperation(const FName& OperationName);
};
