// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NProcGenDeveloperOverlayWidget.generated.h"

class UNListView;

UCLASS(BlueprintType, Abstract)
class UNProcGenDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
	
protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> OperationsList;	
};
