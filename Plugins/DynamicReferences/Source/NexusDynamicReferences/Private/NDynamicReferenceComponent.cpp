// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferenceComponent.h"
#include "NDynamicReferenceSubsystem.h"

void UNDynamicReferenceComponent::BeginPlay()
{
	for (TArray<AActor*>& ReferencedActors = UNDynamicReferenceSubsystem::Get(GetWorld())->GetReferences(ENDynamicReference::NDR_Item_L);
		AActor*& Actor : ReferencedActors)
	{
		
	}
	
	if (LinkPhase == ENActorComponentLifecycleStart::ACLS_BeginPlay)
	{
		// Register the references with the subsystem
		Register();
	}
	Super::BeginPlay();
	
}

void UNDynamicReferenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BreakPhase == ENActorComponentLifecycleEnd::ACLE_EndPlay)
	{
		Unregister();
	}
	Super::EndPlay(EndPlayReason);
}

void UNDynamicReferenceComponent::InitializeComponent()
{
	if (LinkPhase == ENActorComponentLifecycleStart::ACLS_InitializeComponent)
	{
		Register();
	}
	Super::InitializeComponent();
}

void UNDynamicReferenceComponent::UninitializeComponent()
{
	if (BreakPhase == ENActorComponentLifecycleEnd::ACLE_UninitializeComponent)
	{
		// Unregister the references with the subsystem
		Unregister();
	}
	Super::UninitializeComponent();
}

void UNDynamicReferenceComponent::Register()
{
	const int ReferenceCount = References.Num();
	if (ReferenceCount > 0)
	{
		UNDynamicReferenceSubsystem* Subsystem = UNDynamicReferenceSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		for (int i = 0; i < ReferenceCount; i++)
		{
			if (References[i] == NDR_None) continue;
			Subsystem->RegisterReference(References[i], Owner);
		}
	}
}

void UNDynamicReferenceComponent::Unregister()
{
	const int ReferenceCount = References.Num();
	if (ReferenceCount > 0)
	{
		UNDynamicReferenceSubsystem* Subsystem = UNDynamicReferenceSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		for (int i = 0; i < ReferenceCount; i++)
		{
			if (References[i] == NDR_None) continue;
			Subsystem->UnregisterReference(References[i], Owner);
		}
	}
}
