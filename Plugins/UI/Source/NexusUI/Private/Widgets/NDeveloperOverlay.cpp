// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NDeveloperOverlay.h"

void UNDeveloperOverlay::ShowContainerBanner(const FText& Text, ENColor MessageColor, ENColor BannerColor) const
{
	if (ContainerBanner != nullptr && ContainerBanner->IsValidLowLevel())
	{
		ContainerBanner->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ContainerBanner->SetBrushColor(FNColor::GetLinearColor(BannerColor));
		if (!Text.IsEmpty())
		{
			ContainerBannerMessage->SetText(Text);
			ContainerBannerMessage->SetColorAndOpacity(FNColor::GetLinearColor(MessageColor));
		}
	}
}

void UNDeveloperOverlay::HideContainerBanner() const
{
	ContainerBanner->SetVisibility(ESlateVisibility::Collapsed);
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