// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/SNTexturePacker.h"

#include "TexturePacker/SNTexturePackerPackPanel.h"
#include "TexturePacker/SNTexturePackerUnpackPanel.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

void SNTexturePacker::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(8.0f, 8.0f, 8.0f, 4.0f)
		[
			SNew(SSegmentedControl<ENTexturePackerMode>)
			.Value(this, &SNTexturePacker::GetMode)
			.OnValueChanged(this, &SNTexturePacker::SetMode)

			+ SSegmentedControl<ENTexturePackerMode>::Slot(ENTexturePackerMode::Pack)
			.Text(LOCTEXT("Mode_Pack", "Pack"))
			.ToolTip(LOCTEXT("Mode_Pack_Tooltip", "Combine up to four maps into the channels of one texture."))

			+ SSegmentedControl<ENTexturePackerMode>::Slot(ENTexturePackerMode::Unpack)
			.Text(LOCTEXT("Mode_Unpack", "Unpack"))
			.ToolTip(LOCTEXT("Mode_Unpack_Tooltip", "Split a packed texture back into one asset per map it holds."))
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(Switcher, SWidgetSwitcher)
			.WidgetIndex(this, &SNTexturePacker::GetModeIndex)

			+ SWidgetSwitcher::Slot()
			[
				SAssignNew(PackPanel, SNTexturePackerPackPanel)
			]

			+ SWidgetSwitcher::Slot()
			[
				SAssignNew(UnpackPanel, SNTexturePackerUnpackPanel)
			]
		]
	];
}

void SNTexturePacker::OpenForUnpack(UTexture2D* Texture)
{
	SetMode(ENTexturePackerMode::Unpack);

	if (UnpackPanel.IsValid())
	{
		UnpackPanel->SetSourceTexture(Texture);
	}
}

void SNTexturePacker::OpenForPack(const TArray<UTexture2D*>& Textures)
{
	SetMode(ENTexturePackerMode::Pack);

	if (PackPanel.IsValid())
	{
		PackPanel->SetSourceTextures(Textures);
	}
}

void SNTexturePacker::SetMode(const ENTexturePackerMode InMode)
{
	Mode = InMode;
}

int32 SNTexturePacker::GetModeIndex() const
{
	return static_cast<int32>(Mode);
}

#undef LOCTEXT_NAMESPACE
