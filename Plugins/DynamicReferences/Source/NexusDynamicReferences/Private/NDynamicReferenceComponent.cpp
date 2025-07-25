// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferenceComponent.h"
#include "NDynamicReferenceSubsystem.h"

void UNDynamicReferenceComponent::BeginPlay()
{

	if (LinkPhase == ENActorComponentLifecycleStart::BeginPlay)
	{
		// Register the references with the subsystem
		Register();
	}
	Super::BeginPlay();
	
}

void UNDynamicReferenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BreakPhase == ENActorComponentLifecycleEnd::EndPlay)
	{
		Unregister();
	}
	Super::EndPlay(EndPlayReason);
}

void UNDynamicReferenceComponent::InitializeComponent()
{
	if (LinkPhase == ENActorComponentLifecycleStart::InitializeComponent)
	{
		Register();
	}
	Super::InitializeComponent();
}

void UNDynamicReferenceComponent::UninitializeComponent()
{
	if (BreakPhase == ENActorComponentLifecycleEnd::UninitializeComponent)
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
