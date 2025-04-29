// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTextRenderActor.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"

ANTextRenderActor::ANTextRenderActor(const FObjectInitializer& Initializer) : Super(Initializer)
{
	bReplicates = true;
}

void ANTextRenderActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANTextRenderActor, CachedValue);
}

void ANTextRenderActor::OnRep_TextValue()
{
	GetTextRender()->SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void ANTextRenderActor::SetFromName_Implementation(const FName& NewValue)
{
	FString NewString = NewValue.ToString();
	if (CachedValue.Equals(NewString))
	{
		return;
	}
	CachedValue = NewString;
	GetTextRender()->SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void ANTextRenderActor::SetFromString_Implementation(const FString& NewValue)
{
	if (CachedValue.Equals(NewValue))
	{
		return;
	}
	
	CachedValue = NewValue;
	GetTextRender()->SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void ANTextRenderActor::SetFromText_Implementation(const FText& NewValue)
{
	if (NewValue.EqualTo(FText::FromString(CachedValue)))
	{
		return;
	}
	CachedValue = NewValue.ToString();
	GetTextRender()->SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}


