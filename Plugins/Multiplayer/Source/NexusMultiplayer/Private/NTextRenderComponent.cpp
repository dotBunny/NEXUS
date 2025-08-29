// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTextRenderComponent.h"

#include "NCoreMinimal.h"
#include "NMultiplayerUtils.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UNTextRenderComponent::UNTextRenderComponent(const FObjectInitializer& Initializer)
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
	
	CachedValue = Text.ToString();
}

void UNTextRenderComponent::OnRep_TextValue()
{
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::WorldAuthority_SetFromName(const FName& NewValue)
{
	if (!FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
	{
		N_LOG(Warning, TEXT("NO AUTHORITY"))
		return;
	}
	
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

void UNTextRenderComponent::WorldAuthority_SetFromString(const FString& NewValue)
{
	if (!FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
	{
		N_LOG(Warning, TEXT("NO AUTHORITY"))
		return;
	}

	if (CachedValue.Equals(NewValue))
	{
		return;
	}
	
	CachedValue = NewValue;
	MARK_PROPERTY_DIRTY_FROM_NAME(UNTextRenderComponent, CachedValue, this);
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::WorldAuthority_SetFromText(const FText& NewValue)
{
	if (!FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
	{
		N_LOG(Warning, TEXT("NO AUTHORITY"))
		return;
	}
	
	if (NewValue.EqualTo(FText::FromString(CachedValue)))
	{
		return;
	}
	CachedValue = NewValue.ToString();
	MARK_PROPERTY_DIRTY_FROM_NAME(UNTextRenderComponent, CachedValue, this);
	SetText(FText::FromString(CachedValue));
	OnTextChanged.Broadcast(CachedValue);
}

void UNTextRenderComponent::Server_SetFromName_Implementation(const FName& NewValue)
{
	WorldAuthority_SetFromName(NewValue);
}

void UNTextRenderComponent::Server_SetFromString_Implementation(const FString& NewValue)
{
	WorldAuthority_SetFromString(NewValue);
}

void UNTextRenderComponent::Server_SetFromText_Implementation(const FText& NewValue)
{
	WorldAuthority_SetFromText(NewValue);
}
