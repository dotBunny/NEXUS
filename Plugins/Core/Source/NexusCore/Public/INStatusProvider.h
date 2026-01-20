// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INStatusProvider.generated.h"

UINTERFACE(BlueprintType)
class NEXUSCORE_API UNStatusProvider : public UInterface
{
	GENERATED_BODY()
};

class NEXUSCORE_API INStatusProvider
{
	GENERATED_BODY()

public:
	
	static FString InvokeGetStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNStatusProvider>())
		{
			const INStatusProvider* StatusProvider = Cast<INStatusProvider>(BaseWidget);
			if (StatusProvider != nullptr) 
			{
				return StatusProvider->GetStatusProviderMessage();
			}
			return Execute_OnGetStatusProviderMessage(BaseWidget);
		}
		return TEXT("");
	}
	
	static bool InvokeHasStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNStatusProvider>())
		{
			const INStatusProvider* StatusProvider = Cast<INStatusProvider>(BaseWidget);
			if (StatusProvider != nullptr) 
			{
				return StatusProvider->HasStatusProviderMessage();
			}
			return Execute_OnHasStatusProviderMessage(BaseWidget);
		}
		return false;
	}
	

	virtual bool HasStatusProviderMessage() const { return false; }
	virtual FString GetStatusProviderMessage() const { return TEXT(""); }
	
	UFUNCTION(BlueprintImplementableEvent)
	FString OnGetStatusProviderMessage();
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnHasStatusProviderMessage();
};