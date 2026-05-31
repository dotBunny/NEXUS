// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SceneComponents/NTextRenderComponent.h"

#include "NUIMinimal.h"
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
		UE_LOG(LogNexusUI, Warning, TEXT("NTextRenderComponent(%s) turned on replication for its Owner(%s); was this a mistake?"), *GetName(), *GetOwner()->GetActorNameOrLabel());
		GetOwner()->SetReplicates(true);
	}

	if (!IsRunningDedicatedServer())
	{
		CachedValue = Text.ToString();
	}
}

void UNTextRenderComponent::OnRep_TextValue()
{
	if (!IsRunningDedicatedServer())
	{
		SetText(FText::FromString(CachedValue));
	}
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

		if (!IsRunningDedicatedServer())
		{
			SetText(FText::FromString(CachedValue));
		}
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

		if (!IsRunningDedicatedServer())
		{
			SetText(FText::FromString(CachedValue));
		}
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

		if (!IsRunningDedicatedServer())
		{
			SetText(FText::FromString(CachedValue));
		}
		OnTextChanged.Broadcast(CachedValue);
	}
}