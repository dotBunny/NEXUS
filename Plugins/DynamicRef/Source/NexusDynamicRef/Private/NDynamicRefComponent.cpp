// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefComponent.h"
#include "NDynamicRefSubsystem.h"

void UNDynamicRefComponent::BeginPlay()
{
	if (LinkPhase == ENActorComponentLifecycleStart::BeginPlay)
	{
		// Register the references with the subsystem
		Register();
	}
	Super::BeginPlay();
	
}

void UNDynamicRefComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BreakPhase == ENActorComponentLifecycleEnd::EndPlay)
	{
		Unregister();
	}
	Super::EndPlay(EndPlayReason);
}

void UNDynamicRefComponent::InitializeComponent()
{
	if (LinkPhase == ENActorComponentLifecycleStart::InitializeComponent)
	{
		Register();
	}
	Super::InitializeComponent();
}

void UNDynamicRefComponent::UninitializeComponent()
{
	if (BreakPhase == ENActorComponentLifecycleEnd::UninitializeComponent)
	{
		// Unregister the references with the subsystem
		Unregister();
	}
	Super::UninitializeComponent();
}

void UNDynamicRefComponent::Register()
{
	const int FastReferenceCount = FastReferences.Num();
	const int NamedReferenceCount = NamedReferences.Num();
	if (FastReferenceCount > 0 || NamedReferenceCount > 0)
	{
		UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		for (int i = 0; i < FastReferenceCount; i++)
		{
			if (FastReferences[i] == NDR_None) continue;
			Subsystem->AddFastReference(FastReferences[i], Owner);
		}
		for (int i = 0; i < NamedReferenceCount; i++)
		{
			if (NamedReferences[i] == NAME_None) continue;
			Subsystem->AddNamedReference(NamedReferences[i], Owner);
		}
	}
}

void UNDynamicRefComponent::Unregister()
{	
	const int FastReferenceCount = FastReferences.Num();
	const int NamedReferenceCount = NamedReferences.Num();
	if (FastReferenceCount > 0 || NamedReferenceCount > 0)
	{
		UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		for (int i = 0; i < FastReferenceCount; i++)
		{
			if (FastReferences[i] == NDR_None) continue;
			Subsystem->RemoveFastReference(FastReferences[i], Owner);
		}
		for (int i = 0; i < NamedReferenceCount; i++)
		{
			if (NamedReferences[i] == NAME_None) continue;
			Subsystem->RemoveNamedReference(NamedReferences[i], Owner);
		}
	}
}
