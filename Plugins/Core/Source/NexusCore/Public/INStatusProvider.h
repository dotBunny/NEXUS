// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INStatusProvider.generated.h"


#define N_BIND_STATUS_PROVIDER_UPDATED(BaseWidget, Handle, Method) \
	if (BaseWidget->Implements<UNStatusProvider>()) \
	{ \
		INStatusProvider* StatusProvider = Cast<INStatusProvider>(BaseWidget); \
		if (StatusProvider != nullptr) \
		{ \
			Handle = StatusProvider->OnStatusProviderUpdated().AddUObject(this, &Method); \
		} \
	}

#define N_UNBIND_STATUS_PROVIDER_UPDATED(BaseWidget, Handle) \
		if (BaseWidget->Implements<UNStatusProvider>()) \
		{ \
			INStatusProvider* StatusProvider = Cast<INStatusProvider>(BaseWidget); \
			if (StatusProvider != nullptr) \
			{ \
				StatusProvider->OnStatusProviderUpdated().Remove(Handle); \
			} \
		}

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

	DECLARE_EVENT(INStatusProvider, FStatusProviderUpdated);
	FStatusProviderUpdated& OnStatusProviderUpdated() { return StatusProviderUpdatedEvent; }
	void BroadcastStatusProviderUpdated() const { StatusProviderUpdatedEvent.Broadcast(); }
	
	UFUNCTION(BlueprintImplementableEvent)
	FString OnGetStatusProviderMessage();
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnHasStatusProviderMessage();
	
private:
	FStatusProviderUpdated StatusProviderUpdatedEvent;
};