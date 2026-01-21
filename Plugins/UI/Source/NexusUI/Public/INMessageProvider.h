// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INMessageProvider.generated.h"


#define N_BIND_MESSAGE_PROVIDER_UPDATED(BaseWidget, Handle, Method) \
	if (BaseWidget->Implements<UNMessageProvider>()) \
	{ \
		INMessageProvider* MessageProvider = Cast<INMessageProvider>(BaseWidget); \
		if (MessageProvider != nullptr) \
		{ \
			Handle = MessageProvider->OnProvidedMessageChanged().AddUObject(this, &Method); \
		} \
	}

#define N_UNBIND_MESSAGE_PROVIDER_UPDATED(BaseWidget, Handle) \
		if (BaseWidget->Implements<UNMessageProvider>()) \
		{ \
			INMessageProvider* MessageProvider = Cast<INMessageProvider>(BaseWidget); \
			if (MessageProvider != nullptr) \
			{ \
				MessageProvider->OnProvidedMessageChanged().Remove(Handle); \
			} \
		}

UINTERFACE(BlueprintType)
class NEXUSUI_API UNMessageProvider : public UInterface
{
	GENERATED_BODY()
};

class NEXUSUI_API INMessageProvider
{
	GENERATED_BODY()

public:
	
	static FString InvokeGetStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNMessageProvider>())
		{
			const INMessageProvider* MessageProvider = Cast<INMessageProvider>(BaseWidget);
			if (MessageProvider != nullptr) 
			{
				return MessageProvider->GetProvidedMessage();
			}
			return Execute_OnGetProvidedMessage(BaseWidget);
		}
		return TEXT("");
	}
	
	static bool InvokeHasStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNMessageProvider>())
		{
			const INMessageProvider* MessageProvider = Cast<INMessageProvider>(BaseWidget);
			if (MessageProvider != nullptr) 
			{
				return MessageProvider->HasProvidedMessage();
			}
			return Execute_OnHasProvidedMessage(BaseWidget);
		}
		return false;
	}
	
	virtual bool HasProvidedMessage() const { return false; }
	virtual FString GetProvidedMessage() const { return TEXT(""); }

	DECLARE_EVENT(INMessageProvider, FProvidedMessageChanged);
	FProvidedMessageChanged& OnProvidedMessageChanged() { return OnProvidedMessageChangedEvent; }
	void BroadcastProvidedMessageChanged() const { OnProvidedMessageChangedEvent.Broadcast(); }
	
	UFUNCTION(BlueprintImplementableEvent)
	FString OnGetProvidedMessage();
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnHasProvidedMessage();
	
private:
	FProvidedMessageChanged OnProvidedMessageChangedEvent;
};