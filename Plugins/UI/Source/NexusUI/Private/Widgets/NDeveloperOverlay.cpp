// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NDeveloperOverlay.h"

#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NUIMinimal.h"
#include "Macros/NValidationMacros.h"

void UNDeveloperOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	// ContainerBanner is optional for delegate wiring; validate only to log a message so a designer
	// realizes they haven't hooked up the widget. This is intentionally not an early-out.
	N_VALIDATE(LogNexusUI, ContainerBanner);

	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
		this, &UNDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(
		this, &UNDeveloperOverlay::OnWorldBeginTearDown);

	BindAllCurrentWorlds();
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
		if (UWorld* World = Context.World())
		{
			BindWorld(World);
		}
	}
}

void UNDeveloperOverlay::UnbindAllCurrentWorlds()
{
	if (GEngine == nullptr) return;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (const UWorld* World = Context.World())
		{
			UnbindWorld(World);
		}
	}
}