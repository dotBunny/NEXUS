// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefComponent.h"

#include "NDynamicRefsMinimal.h"
#include "NDynamicRefSubsystem.h"

FString UNDynamicRefComponent::ToStringSlow(const ENDynamicRef& DynamicReference)
{
	return StaticEnum<ENDynamicRef>()->GetDisplayNameTextByValue(DynamicReference).ToString();
}

UNDynamicRefComponent::UNDynamicRefComponent()
{
	// InitializeComponent() is gated by bWantsInitializeComponent, which the engine reads during its
	// InitializeComponents() pass. OnComponentCreated() is NOT called for level-placed (serialized)
	// components, so setting the flag there leaves placed actors using the InitializeComponent
	// lifecycle unregistered. Set it unconditionally here (runs for both loaded and spawned
	// components); InitializeComponent()/UninitializeComponent() already gate the actual work on Lifecycle.
	bWantsInitializeComponent = true;
}

void UNDynamicRefComponent::BeginPlay()
{
	if (Lifecycle == ENActorComponentLifecycle::BeginPlay)
	{
		Register();
	}
	Super::BeginPlay();
}

void UNDynamicRefComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Lifecycle == ENActorComponentLifecycle::BeginPlay)
	{
		Unregister();
	}
	Super::EndPlay(EndPlayReason);
}

void UNDynamicRefComponent::InitializeComponent()
{
	if (Lifecycle == ENActorComponentLifecycle::InitializeComponent)
	{
		Register();
	}
	Super::InitializeComponent();
}

void UNDynamicRefComponent::UninitializeComponent()
{
	if (Lifecycle == ENActorComponentLifecycle::InitializeComponent)
	{
		Unregister();
	}
	Super::UninitializeComponent();
}

void UNDynamicRefComponent::Register()
{
	const int32 FastReferenceCount = FastReferences.Num();
	const int32 NamedReferenceCount = NamedReferences.Num();
	const int32 TagReferenceCount = TagReferences.Num();
	if (FastReferenceCount > 0 || NamedReferenceCount > 0 || TagReferenceCount > 0)
	{
		UWorld* World = GetWorld();
		AActor* Owner = GetOwner();
		UNDynamicRefSubsystem* Subsystem = World ? UNDynamicRefSubsystem::Get(World) : nullptr;

		if (Subsystem == nullptr)
		{
			UE_LOG(LogNexusDynamicRefs, Error, TEXT("Failed to register NDynamicRefComponent(%s) as no UNDynamicRefSubsystem was found."), *GetNameSafe(Owner));
			return;
		}

		for (int32 i = 0; i < FastReferenceCount; i++)
		{
			if (FastReferences[i] == NDR_None) continue;
			Subsystem->AddObject(FastReferences[i], Owner);
		}
		for (int32 i = 0; i < NamedReferenceCount; i++)
		{
			if (NamedReferences[i] == NAME_None) continue;
			Subsystem->AddObjectByName(NamedReferences[i], Owner);
		}

		// Store gameplay tags as if they are named
		TArray<FGameplayTag> GameplayTags;
		TagReferences.GetGameplayTagArray(GameplayTags);
		for (int32 i = 0; i < TagReferenceCount; i++)
		{
			if (GameplayTags[i] == FGameplayTag::EmptyTag) continue;
			Subsystem->AddObjectByName(GameplayTags[i].GetTagName(), Owner);
		}
	}
}

void UNDynamicRefComponent::Unregister()
{
	const int32 FastReferenceCount = FastReferences.Num();
	const int32 NamedReferenceCount = NamedReferences.Num();
	const int32 TagReferenceCount = TagReferences.Num();

	if (FastReferenceCount > 0 || NamedReferenceCount > 0 || TagReferenceCount > 0)
	{
		UWorld* World = GetWorld();
		AActor* Owner = GetOwner();
		UNDynamicRefSubsystem* Subsystem = World ? UNDynamicRefSubsystem::Get(World) : nullptr;

		if (Subsystem == nullptr)
		{
			UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Failed to unregister NDynamicRefComponent(%s) as no UNDynamicRefSubsystem was found."), *GetNameSafe(Owner));
			return;
		}

		for (int32 i = 0; i < FastReferenceCount; i++)
		{
			if (FastReferences[i] == NDR_None) continue;
			Subsystem->RemoveObject(FastReferences[i], Owner);
		}
		for (int32 i = 0; i < NamedReferenceCount; i++)
		{
			if (NamedReferences[i] == NAME_None) continue;
			Subsystem->RemoveObjectByName(NamedReferences[i], Owner);
		}

		// Store gameplay tags as if they are named
		TArray<FGameplayTag> GameplayTags;
		TagReferences.GetGameplayTagArray(GameplayTags);
		for (int32 i = 0; i < TagReferenceCount; i++)
		{
			if (GameplayTags[i] == FGameplayTag::EmptyTag) continue;
			Subsystem->RemoveObjectByName(GameplayTags[i].GetTagName(), Owner);
		}
	}
}
