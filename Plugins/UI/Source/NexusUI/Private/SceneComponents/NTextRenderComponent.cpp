// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SceneComponents/NTextRenderComponent.h"

#include "NMultiplayerUtils.h"
#include "NUIMinimal.h"
#include "Iris/ReplicationSystem/Prioritization/SphereWithOwnerBoostNetObjectPrioritizer.h"
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

	// Replicated text only propagates if the owning actor itself replicates. Either force replication
	// on for the owner, or warn rather than silently failing to propagate.
	AActor* Owner = GetOwner();
	if (bShouldCheckReplication && Owner != nullptr && FNMultiplayerUtils::HasWorldAuthority(GetWorld()) && !Owner->GetIsReplicated())
	{
		if (bForceOwnerReplication)
		{
			Owner->SetReplicates(true);
			UE_LOG(LogNexusUI, Log, TEXT("NTextRenderComponent(%s)'s Owner(%s) was not replicated; replication has been forced on so replicated text can propagate. Disable bForceOwnerReplication to opt out of this behavior."), *GetName(), *Owner->GetActorNameOrLabel());
		}
		else
		{
			UE_LOG(LogNexusUI, Warning, TEXT("NTextRenderComponent(%s)'s Owner(%s) is not replicated, so replicated text will not propagate. Enable replication on the owning actor, or disable bShouldCheckReplication on this component."), *GetName(), *Owner->GetActorNameOrLabel());
		}
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
	if (FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
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
	if (FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
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
	if (FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
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