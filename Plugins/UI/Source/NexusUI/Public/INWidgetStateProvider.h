// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWidgetState.h"
#include "INWidgetStateProvider.generated.h"

UINTERFACE(BlueprintType)
class NEXUSUI_API UNWidgetStateProvider : public UInterface
{
	GENERATED_BODY()
};


class NEXUSUI_API INWidgetStateProvider
{
	GENERATED_BODY()

public:
	
	virtual void RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& InState)
	{
		Execute_OnRestoreWidgetStateEvent(BlueprintWidget, Identifier, InState);
		bHasWidgetStateBeenRestored = true;
	};
	
	virtual FNWidgetState GetWidgetState(UObject* BlueprintWidget)
	{
		return Execute_OnGetWidgetStateEvent(BlueprintWidget);
	};
	
	bool HasWidgetStateBeenRestored() const
	{
		return bHasWidgetStateBeenRestored;
	}
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRestoreWidgetStateEvent(const FName& Identifier, FNWidgetState State);
	
	UFUNCTION(BlueprintImplementableEvent)
	FNWidgetState OnGetWidgetStateEvent();
	
protected:
	bool bHasWidgetStateBeenRestored = false;
};