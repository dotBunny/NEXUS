// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NDeveloperOverlay.h"

void UNDeveloperOverlay::ShowBannerMessage(const FText& Text, ENColor MessageColor, ENColor BannerColor) const
{
	if (ContainerBanner != nullptr && ContainerBanner->IsValidLowLevel())
	{
		ContainerBanner->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (!Text.IsEmpty())
		{
			ContainerBannerMessage->SetText(Text);
			ContainerBannerMessage->SetColorAndOpacity(FNColor::GetLinearColor(MessageColor));
			ContainerBanner->SetBrushColor(FNColor::GetLinearColor(BannerColor));
		}
	}
}

void UNDeveloperOverlay::HideBanner() const
{
	ContainerBanner->SetVisibility(ESlateVisibility::Collapsed);
}