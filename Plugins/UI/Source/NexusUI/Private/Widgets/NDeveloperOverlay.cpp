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