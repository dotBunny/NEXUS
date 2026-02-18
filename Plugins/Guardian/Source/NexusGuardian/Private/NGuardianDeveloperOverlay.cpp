// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianDeveloperOverlay.h"

#include "NGuardianSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/HorizontalBox.h"

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
	if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
	{
		UNGuardianSubsystem* System = UNGuardianSubsystem::Get(World);
		if (System != nullptr)
		{
			Subsystems.AddUnique(System);
		}
	}
	UpdateBanner();
}

void UNGuardianDeveloperOverlay::Unbind(const UWorld* World)
{
	if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
	{
		UNGuardianSubsystem* System = UNGuardianSubsystem::Get(World);
		if (System != nullptr)
		{
			Subsystems.Remove(System);
		}
	}
	UpdateBanner();
}

void UNGuardianDeveloperOverlay::OnWorldPostInitialization(UWorld* World, 
	FWorldInitializationValues WorldInitializationValues)
{
	if (World != nullptr)
	{
		Bind(World);
	}
}

void UNGuardianDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
	if (World != nullptr)
	{
		Unbind(World);
	}
}


void UNGuardianDeveloperOverlay::UpdateBanner() const
{
	if (Subsystems.Num() <= 0)
	{
		ShowContainerBanner(RuntimeOnlyText,
				UNStyleLibrary::GetInfoForegroundColor(), 
				UNStyleLibrary::GetInfoBackgroundColor());
		
		if (ObjectCountBox != nullptr)
		{
			ObjectCountBox->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		HideContainerBanner();
		
		if (ObjectCountBox != nullptr)
		{
			ObjectCountBox->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UNGuardianDeveloperOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if (Subsystems.Num() <= 0) return;

	UNGuardianSubsystem* System = Subsystems[0];
	
	ObjectCountNumber->SetCurrentValue(System->GetLastObjectCount());
	BaseCountNumber->SetCurrentValue(System->GetBaseObjectCount());
	
	if (System->HasPassedCompareThreshold())
	{
		ObjectCountNumber->SetColorAndOpacity(FLinearColor::Blue);
		
		ObjectCountNextNumber->SetCurrentValue(0);
		ObjectCountNextNumber->SetColorAndOpacity(FLinearColor::Blue);
	}
	else if (System->HasPassedSnapshotThreshold())
	{
		ObjectCountNumber->SetColorAndOpacity(FLinearColor::Red);
		
		ObjectCountNextNumber->SetCurrentValue(System->GetObjectCountCompareThreshold());
		ObjectCountNextNumber->SetColorAndOpacity(FLinearColor::Red);
	}
	else if (System->HasPassedWarningThreshold())
	{
		ObjectCountNumber->SetColorAndOpacity(FLinearColor::Yellow);
		
		ObjectCountNextNumber->SetCurrentValue(System->GetObjectCountSnapshotThreshold());
		ObjectCountNextNumber->SetColorAndOpacity(FLinearColor::Yellow);
	}
	else
	{
		ObjectCountNumber->SetColorAndOpacity(FLinearColor::White);
		
		ObjectCountNextNumber->SetCurrentValue(System->GetObjectCountWarningThreshold());
		ObjectCountNextNumber->SetColorAndOpacity(FLinearColor::White);
	}
	
	Super::NativeTick(MyGeometry, InDeltaTime);
}
