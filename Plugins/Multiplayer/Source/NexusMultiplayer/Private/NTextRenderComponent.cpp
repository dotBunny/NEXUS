// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTextRenderComponent.h"
#include "Net/UnrealNetwork.h"

UNTextRenderComponent::UNTextRenderComponent(const FObjectInitializer& Initializer)
{
	//PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UNTextRenderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNTextRenderComponent, CachedValue);
}

void UNTextRenderComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedValue = Text.ToString();
}

void UNTextRenderComponent::OnRep_TextValue()
{
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::SetFromName_Implementation(const FName& NewValue)
{
	FString NewString = NewValue.ToString();
	if (CachedValue.Equals(NewString))
	{
		return;
	}
	CachedValue = NewString;
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::SetFromString_Implementation(const FString& NewValue)
{
	if (CachedValue.Equals(NewValue))
	{
		return;
	}
	
	CachedValue = NewValue;
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::SetFromText_Implementation(const FText& NewValue)
{
	if (NewValue.EqualTo(FText::FromString(CachedValue)))
	{
		return;
	}
	CachedValue = NewValue.ToString();
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}
