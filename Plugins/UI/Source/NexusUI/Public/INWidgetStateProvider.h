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
	
	static void InvokeRestoreWidgetState(UObject* BaseWidget, const FName Identifier, FNWidgetState& WidgetState)
	{
		if (BaseWidget->Implements<UNWidgetStateProvider>())
		{
			INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(BaseWidget);
			if (StateProvider != nullptr) 
			{
				StateProvider->RestoreWidgetState(BaseWidget, Identifier, WidgetState);
			}
			else
			{
				Execute_OnRestoreWidgetStateEvent(BaseWidget, Identifier, WidgetState);
			}
		}
	}
	
	static FNWidgetState InvokeGetWidgetState(UObject* BlueprintWidget)
	{
		if (BlueprintWidget->Implements<UNWidgetStateProvider>())
		{
			INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(BlueprintWidget); 
			if (StateProvider != nullptr)
			{
				return StateProvider->GetWidgetState(BlueprintWidget);
			}
			return Execute_OnGetWidgetStateEvent(BlueprintWidget);
		}
		return FNWidgetState();
	}
	
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
	void SetWidgetStateHasBeenRestored(const bool bHasBeenRestored)
	{
		bHasWidgetStateBeenRestored = bHasBeenRestored;
	}
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRestoreWidgetStateEvent(const FName& Identifier, FNWidgetState State);
	
	UFUNCTION(BlueprintImplementableEvent)
	FNWidgetState OnGetWidgetStateEvent();
	
private:
	bool bHasWidgetStateBeenRestored = false;
};