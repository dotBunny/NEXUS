// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefComponent.h"

#include "NDynamicRefsMinimal.h"
#include "NDynamicRefSubsystem.h"

void UNDynamicRefComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	bWantsInitializeComponent = (Lifecycle == ENActorComponentLifecycle::InitializeComponent);
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
	if (FastReferenceCount > 0 || NamedReferenceCount > 0)
	{
		UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		
		if (Subsystem == nullptr)
		{
			UE_LOG(LogNexusDynamicRefs, Error, TEXT("Failed to register NDynamicRefComponent(%s) as no UNDynamicRefSubsystem was found."), *Owner->GetName());
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
	}
}

void UNDynamicRefComponent::Unregister()
{	
	const int32 FastReferenceCount = FastReferences.Num();
	const int32 NamedReferenceCount = NamedReferences.Num();
	if (FastReferenceCount > 0 || NamedReferenceCount > 0)
	{
		UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(GetWorld());
		AActor* Owner = GetOwner();
		
		if (Subsystem == nullptr)
		{
			UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Failed to unregister NDynamicRefComponent(%s) as no UNDynamicRefSubsystem was found."), *Owner->GetName());
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
	}
}
