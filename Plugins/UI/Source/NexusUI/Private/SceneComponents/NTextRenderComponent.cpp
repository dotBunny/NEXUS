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

	// Replicated text only propagates if the owning actor itself replicates; warn rather than silently
	// mutating the owner's network behavior.
	const AActor* Owner = GetOwner();
	if (bShouldCheckReplication && Owner != nullptr && Owner->HasAuthority() && !Owner->GetIsReplicated())
	{
		UE_LOG(LogNexusUI, Error, TEXT("NTextRenderComponent(%s)'s Owner(%s) is not replicated, so replicated text will not propagate. Enable replication on the owning actor, or disable bShouldCheckReplication on this component."), *GetName(), *Owner->GetActorNameOrLabel());
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