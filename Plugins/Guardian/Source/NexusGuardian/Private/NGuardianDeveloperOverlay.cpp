// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianDeveloperOverlay.h"

#include "NGuardianSubsystem.h"

void UNGuardianDeveloperOverlay::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNGuardianDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNGuardianDeveloperOverlay::OnWorldBeginTearDown);
	
	// Look at all worlds and add them to bindings
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const auto& Context : WorldContexts)
	{
		Bind(Context.World());
	}

	Super::NativeConstruct();
}

void UNGuardianDeveloperOverlay::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const auto& Context : WorldContexts)
	{
		Unbind(Context.World());
	}
	
	Super::NativeDestruct();
}

void UNGuardianDeveloperOverlay::Bind(UWorld* World)
{
	UNGuardianSubsystem* System = UNGuardianSubsystem::Get(World);
	if (System == nullptr)
	{
		UpdateBanner();
		return; // System-less world
	}
	
	
}

void UNGuardianDeveloperOverlay::Unbind(const UWorld* World)
{
}

void UNGuardianDeveloperOverlay::OnWorldPostInitialization(UWorld* World,
	FWorldInitializationValues WorldInitializationValues)
{
	Bind(World);
}

void UNGuardianDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
}

void UNGuardianDeveloperOverlay::UpdateBanner() const
{
}
