// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTextRenderComponent.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UNTextRenderComponent::UNTextRenderComponent(const FObjectInitializer& Initializer) : Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = false;
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
	
	// Ensure an owner is replicating
	if (!GetOwner()->GetIsReplicated())
	{
		GetOwner()->SetReplicates(true);
	}
	
	CachedValue = Text.ToString();
}

void UNTextRenderComponent::OnRep_TextValue()
{
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::SetFromName(const FName& NewValue)
{
	if (GetOwner()->HasAuthority())
	{
		FString NewString = NewValue.ToString();
		
		if (CachedValue.Equals(NewString))
		{
			return;
		}
		
		CachedValue = NewString;
		MARK_PROPERTY_DIRTY_FROM_NAME(UNTextRenderComponent, CachedValue, this);
		
		SetText(FText::FromString(CachedValue));
		OnTextChanged.Broadcast(CachedValue);
	}
}

void UNTextRenderComponent::SetFromString(const FString& NewValue)
{
	if (GetOwner()->HasAuthority())
	{
		if (CachedValue.Equals(NewValue))
		{
			return;
		}
	
		CachedValue = NewValue;
		MARK_PROPERTY_DIRTY_FROM_NAME(UNTextRenderComponent, CachedValue, this);
		
		SetText(FText::FromString(CachedValue));
		OnTextChanged.Broadcast(CachedValue);
	}
}

void UNTextRenderComponent::SetFromText(const FText& NewValue)
{
	if (GetOwner()->HasAuthority())
	{
		if (NewValue.EqualTo(FText::FromString(CachedValue)))
		{
			return;
		}
		
		CachedValue = NewValue.ToString();
		MARK_PROPERTY_DIRTY_FROM_NAME(UNTextRenderComponent, CachedValue, this);
		
		SetText(FText::FromString(CachedValue));
		OnTextChanged.Broadcast(CachedValue);
	}
}