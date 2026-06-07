// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NDeveloperOverlay.h"

#include "NUIMinimal.h"
#include "Macros/NValidationMacros.h"

void UNDeveloperOverlay::NativeConstruct()
{
	N_VALIDATE(LogNexusUI, ContainerBanner);

	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
		this, &UNDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(
		this, &UNDeveloperOverlay::OnWorldBeginTearDown);

	BindAllCurrentWorlds();

	Super::NativeConstruct();
}

void UNDeveloperOverlay::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);

	UnbindAllCurrentWorlds();

	Super::NativeDestruct();
}

void UNDeveloperOverlay::OnWorldPostInitialization(UWorld* World, FWorldInitializationValues)
{
	if (World != nullptr)
	{
		BindWorld(World);
	}
}

void UNDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
	if (World != nullptr)
	{
		UnbindWorld(World);
	}
}

void UNDeveloperOverlay::ShowContainerBanner(const FText& Text, ENColor MessageColor, ENColor BannerColor) const
{
	if (IsValid(ContainerBanner))
	{
		ContainerBanner->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ContainerBanner->SetBrushColor(FNColor::GetLinearColor(BannerColor));
		if (!Text.IsEmpty() && IsValid(ContainerBannerMessage))
		{
			ContainerBannerMessage->SetText(Text);
			ContainerBannerMessage->SetColorAndOpacity(FNColor::GetLinearColor(MessageColor));
		}
	}
}

void UNDeveloperOverlay::HideContainerBanner() const
{
	if (IsValid(ContainerBanner))
	{
		ContainerBanner->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNDeveloperOverlay::BindAllCurrentWorlds()
{
	if (GEngine == nullptr) return;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		BindWorld(Context.World());
	}
}

void UNDeveloperOverlay::UnbindAllCurrentWorlds()
{
	if (GEngine == nullptr) return;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UnbindWorld(Context.World());
	}
}