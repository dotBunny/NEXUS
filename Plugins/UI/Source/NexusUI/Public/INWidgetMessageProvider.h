// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INWidgetMessageProvider.generated.h"


#define N_BIND_WIDGET_MESSAGE_PROVIDED(BaseWidget, Handle, Method) \
	if (BaseWidget->Implements<UNWidgetMessageProvider>()) \
	{ \
		INWidgetMessageProvider* WidgetMessageProvider = Cast<INWidgetMessageProvider>(BaseWidget); \
		if (WidgetMessageProvider != nullptr) \
		{ \
			Handle = WidgetMessageProvider->OnProvidedMessageChanged().AddUObject(this, &Method); \
		} \
	}

#define N_UNBIND_WIDGET_MESSAGE_PROVIDED(BaseWidget, Handle) \
		if (BaseWidget->Implements<UNWidgetMessageProvider>()) \
		{ \
			INWidgetMessageProvider* WidgetMessageProvider = Cast<INWidgetMessageProvider>(BaseWidget); \
			if (WidgetMessageProvider != nullptr) \
			{ \
				WidgetMessageProvider->OnProvidedMessageChanged().Remove(Handle); \
			} \
		}

UINTERFACE(BlueprintType)
class NEXUSUI_API UNWidgetMessageProvider : public UInterface
{
	GENERATED_BODY()
};

class NEXUSUI_API INWidgetMessageProvider
{
	GENERATED_BODY()

public:
	
	static FString InvokeGetStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNWidgetMessageProvider>())
		{
			const INWidgetMessageProvider* WidgetMessageProvider = Cast<INWidgetMessageProvider>(BaseWidget);
			if (WidgetMessageProvider != nullptr) 
			{
				return WidgetMessageProvider->GetProvidedMessage();
			}
			return Execute_OnGetProvidedMessage(BaseWidget);
		}
		return TEXT("");
	}
	
	static bool InvokeHasStatusProviderMessage(UObject* BaseWidget)
	{
		if (BaseWidget->Implements<UNWidgetMessageProvider>())
		{
			const INWidgetMessageProvider* WidgetMessageProvider = Cast<INWidgetMessageProvider>(BaseWidget);
			if (WidgetMessageProvider != nullptr) 
			{
				return WidgetMessageProvider->HasProvidedMessage();
			}
			return Execute_OnHasProvidedMessage(BaseWidget);
		}
		return false;
	}
	
	virtual bool HasProvidedMessage() const { return false; }
	virtual FString GetProvidedMessage() const { return TEXT(""); }

	DECLARE_EVENT(INWidgetMessageProvider, FProvidedMessageChanged);
	FProvidedMessageChanged& OnProvidedMessageChanged() { return OnProvidedMessageChangedEvent; }
	void BroadcastProvidedMessageChanged() const { OnProvidedMessageChangedEvent.Broadcast(); }
	
	UFUNCTION(BlueprintImplementableEvent)
	FString OnGetProvidedMessage();
	
	UFUNCTION(BlueprintImplementableEvent)
	bool OnHasProvidedMessage();
	
private:
	FProvidedMessageChanged OnProvidedMessageChangedEvent;
};