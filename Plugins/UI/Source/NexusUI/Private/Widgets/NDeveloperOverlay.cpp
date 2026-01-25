// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NDeveloperOverlay.h"

void UNDeveloperOverlay::ShowBannerMessage(const FText& Text, ENColor MessageColor, ENColor BannerColor) const
{
	BannerContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (!Text.IsEmpty())
	{
		BannerMessage->SetText(Text);
		BannerMessage->SetColorAndOpacity(FNColor::GetLinearColor(MessageColor));
		BannerContainer->SetBrushColor(FNColor::GetLinearColor(BannerColor));
	}
}

void UNDeveloperOverlay::HideBanner() const
{
	BannerContainer->SetVisibility(ESlateVisibility::Collapsed);
}