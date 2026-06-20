// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianDeveloperOverlay.h"

#include "NGuardianMinimal.h"
#include "NGuardianSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/HorizontalBox.h"
#include "Macros/NValidationMacros.h"

void UNGuardianDeveloperOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	N_VALIDATE(LogNexusGuardian, ObjectCountBox);
	N_VALIDATE(LogNexusGuardian, ObjectCountNumber);
	N_VALIDATE(LogNexusGuardian, BaseCountNumber);
	N_VALIDATE(LogNexusGuardian, ObjectCountNextNumber);
}

void UNGuardianDeveloperOverlay::BindWorld(UWorld* World)
{
	if (World == nullptr) return;

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

void UNGuardianDeveloperOverlay::UnbindWorld(const UWorld* World)
{
	if (World == nullptr) return;

	if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
	{
		// Remove the world's subsystem if it still resolves, then purge any slots GC has nulled so a
		// dead entry can never linger at index 0 and silently freeze NativeTick.
		if (UNGuardianSubsystem* System = UNGuardianSubsystem::Get(World))
		{
			Subsystems.Remove(System);
		}
		Subsystems.RemoveAll([](const TObjectPtr<UNGuardianSubsystem>& Subsystem){ return Subsystem == nullptr; });
	}
	UpdateBanner();
}

void UNGuardianDeveloperOverlay::UpdateBanner() const
{
	if (Subsystems.IsEmpty())
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
	if (Subsystems.IsEmpty()) return;

	// TODO: This forces us to only show the active game world or PIE, despite the possibility there is more
	// TODO: We should make this a list entry like the others to support multiple worlds
	// Find the first live subsystem rather than assuming index 0; a null slot there would otherwise
	// freeze the overlay while a valid world is still tracked further down the array.
	UNGuardianSubsystem* System = nullptr;
	for (const TObjectPtr<UNGuardianSubsystem>& Candidate : Subsystems)
	{
		if (Candidate != nullptr)
		{
			System = Candidate;
			break;
		}
	}
	if (System == nullptr) return;

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
