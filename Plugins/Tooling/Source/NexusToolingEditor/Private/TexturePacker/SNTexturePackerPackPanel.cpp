// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/SNTexturePackerPackPanel.h"

#include "AssetThumbnail.h"
#include "PropertyCustomizationHelpers.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Texture2D.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "TexturePacker/NTexturePackerJob.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "TexturePacker/NTexturePackerUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker::UI
{
	/** How many thumbnails the window keeps live; four pickers and a little headroom. */
	constexpr int32 ThumbnailPoolSize = 16;

	/** Channel labels, indexed by ENTextureChannel. */
	const TArray<FText>& GetChannelLabels()
	{
		static const TArray<FText> Labels = {
			NSLOCTEXT("NTexturePacker", "Channel_R", "R"),
			NSLOCTEXT("NTexturePacker", "Channel_G", "G"),
			NSLOCTEXT("NTexturePacker", "Channel_B", "B"),
			NSLOCTEXT("NTexturePacker", "Channel_A", "A")
		};
		return Labels;
	}

	/** @return The label for one source-channel choice. */
	FText GetSourceLabel(const ENTextureChannelSource Source)
	{
		switch (Source)
		{
		case ENTextureChannelSource::Auto: return NSLOCTEXT("NTexturePacker", "Source_Auto", "Auto");
		case ENTextureChannelSource::Red: return NSLOCTEXT("NTexturePacker", "Source_Red", "Red");
		case ENTextureChannelSource::Green: return NSLOCTEXT("NTexturePacker", "Source_Green", "Green");
		case ENTextureChannelSource::Blue: return NSLOCTEXT("NTexturePacker", "Source_Blue", "Blue");
		case ENTextureChannelSource::Alpha: return NSLOCTEXT("NTexturePacker", "Source_Alpha", "Alpha");
		case ENTextureChannelSource::Luminance: return NSLOCTEXT("NTexturePacker", "Source_Luminance", "Luminance");
		default: return NSLOCTEXT("NTexturePacker", "Source_Constant", "Constant");
		}
	}

	/** The choices offered for a slot that has a texture; Constant belongs to the slots that do not. */
	const TArray<ENTextureChannelSource>& GetSourceChoices()
	{
		static const TArray<ENTextureChannelSource> Choices = {
			ENTextureChannelSource::Auto,
			ENTextureChannelSource::Red,
			ENTextureChannelSource::Green,
			ENTextureChannelSource::Blue,
			ENTextureChannelSource::Alpha,
			ENTextureChannelSource::Luminance
		};
		return Choices;
	}
}

void SNTexturePackerPackPanel::Construct(const FArguments& InArgs)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UI;

	Slots.SetNum(4);
	ThumbnailPool = MakeShared<FAssetThumbnailPool>(ThumbnailPoolSize);

	// Alpha starts opaque rather than black. A three-channel pack leaves it unused, and an unused alpha that
	// reads as fully transparent is the one value that changes how the texture behaves everywhere else.
	Slots[3].ConstantValue = 1.0f;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	bSixteenBit = Settings->bSixteenBitOutput;

	OutputPath = FNTexturePackerUtils::GetContentBrowserPath();

	const TSharedRef<SGridPanel> ChannelGrid = SNew(SGridPanel).FillColumn(1, 1.0f);

	TSharedRef<SVerticalBox> Channels = SNew(SVerticalBox);
	for (int32 ChannelIndex = 0; ChannelIndex < 4; ChannelIndex++)
	{
		Channels->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				BuildChannelRow(ChannelIndex)
			];
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 8.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("Pack_Explanation",
					"Assign a texture to each channel of the output. What each one holds is read from its "
					"name, and the result is named for the arrangement they make. Sources of different sizes "
					"are resampled onto the largest, and a gamma-encoded source is linearized on the way in."))
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 4.0f)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			[
				Channels
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(this, &SNTexturePackerPackPanel::GetDetectionText)
			.ColorAndOpacity(this, &SNTexturePackerPackPanel::GetDetectionColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			ChannelGrid
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 12.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SNTexturePackerPackPanel::GetSixteenBitState)
				.OnCheckStateChanged(this, &SNTexturePackerPackPanel::HandleSixteenBitChanged)
				.ToolTipText(LOCTEXT("Pack_SixteenBit_Tooltip",
					"Write 16 bits per channel. Worth it where a source was gamma-encoded, since linearizing "
					"it spreads the dark values wider than 8 bits hold."))
				[
					SNew(STextBlock).Text(LOCTEXT("Pack_SixteenBit", "16-bit output"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(this, &SNTexturePackerPackPanel::GetOverwriteState)
				.OnCheckStateChanged(this, &SNTexturePackerPackPanel::HandleOverwriteChanged)
				.ToolTipText(LOCTEXT("Pack_Overwrite_Tooltip",
					"Replace an asset already standing at the output path, checking it out first."))
				[
					SNew(STextBlock).Text(LOCTEXT("Pack_Overwrite", "Overwrite existing"))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(this, &SNTexturePackerPackPanel::GetMessageText)
			.Visibility(this, &SNTexturePackerPackPanel::GetMessageVisibility)
			.ColorAndOpacity(this, &SNTexturePackerPackPanel::GetMessageColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(8.0f, 4.0f, 8.0f, 8.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Pack_Run", "Pack"))
			.ToolTipText(LOCTEXT("Pack_Run_Tooltip", "Write the packed texture. It is left unsaved for review."))
			.IsEnabled(this, &SNTexturePackerPackPanel::CanPack)
			.OnClicked(this, &SNTexturePackerPackPanel::HandlePackClicked)
		]
	];

	// Built after ChildSlot so the grid's rows can reference widgets the panel already owns.
	ChannelGrid->AddSlot(0, 0)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(LOCTEXT("Pack_Path", "Path"))
		];

	ChannelGrid->AddSlot(1, 0)
		.Padding(0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(OutputPathBox, SEditableTextBox)
				.Text(this, &SNTexturePackerPackPanel::GetOutputPathText)
				.OnTextChanged(this, &SNTexturePackerPackPanel::HandleOutputPathChanged)
				.HintText(LOCTEXT("Pack_Path_Hint", "/Game/..."))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Pack_UsePath", "Use Selected"))
				.ToolTipText(LOCTEXT("Pack_UsePath_Tooltip", "Take the folder currently selected in the Content Browser."))
				.OnClicked(this, &SNTexturePackerPackPanel::HandleUseSelectedPathClicked)
			]
		];

	ChannelGrid->AddSlot(0, 1)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(LOCTEXT("Pack_Name", "Name"))
		];

	ChannelGrid->AddSlot(1, 1)
		.Padding(0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(OutputNameBox, SEditableTextBox)
				.Text(this, &SNTexturePackerPackPanel::GetOutputNameText)
				.OnTextChanged(this, &SNTexturePackerPackPanel::HandleOutputNameChanged)
				.HintText(LOCTEXT("Pack_Name_Hint", "T_Example_ORM"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Pack_ResetNaming", "Suggest"))
				.ToolTipText(LOCTEXT("Pack_ResetNaming_Tooltip",
					"Go back to the path and name proposed from the assigned textures."))
				.OnClicked(this, &SNTexturePackerPackPanel::HandleResetNamingClicked)
			]
		];

	RefreshDetection();
}

TSharedRef<SWidget> SNTexturePackerPackPanel::BuildChannelRow(const int32 ChannelIndex)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UI;

	return SNew(SBorder)
		.Padding(6.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(16.0f)
				[
					SNew(STextBlock)
					.Text(GetChannelLabels()[ChannelIndex])
					.Justification(ETextJustify::Center)
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UTexture2D::StaticClass())
				.AllowClear(true)
				.DisplayThumbnail(true)
				.ThumbnailPool(ThumbnailPool)
				.ObjectPath(this, &SNTexturePackerPackPanel::GetSlotObjectPath, ChannelIndex)
				.OnObjectChanged(this, &SNTexturePackerPackPanel::HandleSlotObjectChanged, ChannelIndex)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(90.0f)
				[
					SNew(SComboButton)
					.IsEnabled(this, &SNTexturePackerPackPanel::IsSlotTextureAssigned, ChannelIndex)
					.OnGetMenuContent(this, &SNTexturePackerPackPanel::BuildSourceMenu, ChannelIndex)
					.ToolTipText(LOCTEXT("Pack_Source_Tooltip",
						"Which part of the assigned texture feeds this channel. Auto reads red from a "
						"grayscale source and the matching channel from anything wider."))
					.ButtonContent()
					[
						SNew(STextBlock).Text(this, &SNTexturePackerPackPanel::GetSourceText, ChannelIndex)
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(70.0f)
				.Visibility(this, &SNTexturePackerPackPanel::GetConstantVisibility, ChannelIndex)
				[
					SNew(SSpinBox<float>)
					.MinValue(0.0f)
					.MaxValue(1.0f)
					.Delta(0.01f)
					.Value(this, &SNTexturePackerPackPanel::GetConstantValue, ChannelIndex)
					.OnValueChanged(this, &SNTexturePackerPackPanel::HandleConstantValueChanged, ChannelIndex)
					.ToolTipText(LOCTEXT("Pack_Constant_Tooltip",
						"The value this channel is filled with while no texture is assigned to it."))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsEnabled(this, &SNTexturePackerPackPanel::IsSlotTextureAssigned, ChannelIndex)
				.IsChecked(this, &SNTexturePackerPackPanel::GetInvertState, ChannelIndex)
				.OnCheckStateChanged(this, &SNTexturePackerPackPanel::HandleInvertChanged, ChannelIndex)
				.ToolTipText(LOCTEXT("Pack_Invert_Tooltip",
					"Write one minus the sampled value, which is what turns a gloss map into a roughness one."))
				[
					SNew(STextBlock).Text(LOCTEXT("Pack_Invert", "Invert"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsEnabled(this, &SNTexturePackerPackPanel::IsSlotTextureAssigned, ChannelIndex)
				.IsChecked(this, &SNTexturePackerPackPanel::GetTreatAsLinearState, ChannelIndex)
				.OnCheckStateChanged(this, &SNTexturePackerPackPanel::HandleTreatAsLinearChanged, ChannelIndex)
				.ToolTipText(LOCTEXT("Pack_Raw_Tooltip",
					"Take the source's stored values as they are, ignoring its sRGB flag. Off by default, so "
					"an encoded source is decoded exactly as a material sampler would decode it."))
				[
					SNew(STextBlock).Text(LOCTEXT("Pack_Raw", "Raw"))
				]
			]
		];
}

TSharedRef<SWidget> SNTexturePackerPackPanel::BuildSourceMenu(const int32 ChannelIndex)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UI;

	FMenuBuilder MenuBuilder(true, nullptr);

	for (const ENTextureChannelSource Choice : GetSourceChoices())
	{
		MenuBuilder.AddMenuEntry(
			GetSourceLabel(Choice),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, ChannelIndex, Choice]()
				{
					Slots[ChannelIndex].Source = Choice;
				})),
			NAME_None,
			EUserInterfaceActionType::Button);
	}

	return MenuBuilder.MakeWidget();
}

void SNTexturePackerPackPanel::SetSourceTextures(const TArray<UTexture2D*>& Textures)
{
	// Routed through the same proposal the batch path uses, so what the window opens showing is what an
	// unattended run would have done.
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FNTexturePackRequest Request;
	FNTexturePackerResult Result;

	if (!FNTexturePackerJob::BuildAutoPackRequest(*Settings, Textures, Request, Result))
	{
		Messages = { Result.Error };
		bLastRunFailed = true;
		return;
	}

	for (int32 ChannelIndex = 0; ChannelIndex < 4 && ChannelIndex < Request.Slots.Num(); ChannelIndex++)
	{
		Slots[ChannelIndex].Texture = Request.Slots[ChannelIndex].Texture;
		Slots[ChannelIndex].Source = Request.Slots[ChannelIndex].Source;
	}

	bOutputNameEdited = false;
	bOutputPathEdited = false;
	Messages = Result.Warnings;
	bLastRunFailed = false;

	RefreshDetection();
}

void SNTexturePackerPackPanel::RefreshDetection()
{
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	TArray<FName> Roles;
	Roles.Init(NAME_None, 4);

	TArray<FString> BaseNames;
	const UTexture2D* FirstTexture = nullptr;

	for (int32 ChannelIndex = 0; ChannelIndex < 4; ChannelIndex++)
	{
		const UTexture2D* Texture = Slots[ChannelIndex].Texture.Get();
		if (Texture == nullptr) continue;

		if (FirstTexture == nullptr)
		{
			FirstTexture = Texture;
		}

		const FNTextureNameParts Parts = FNTexturePackerUtils::SplitAssetName(*Settings, Texture->GetName());
		BaseNames.AddUnique(Parts.BaseName);
		Roles[ChannelIndex] = FNTexturePackerUtils::InferRole(*Settings, Texture->GetName());
	}

	bLayoutRecognized = false;
	DetectedLayout = FNTexturePackedLayout();

	if (const FNTexturePackedLayout* Layout = Settings->FindLayoutByRoles(Roles))
	{
		DetectedLayout = *Layout;
		bLayoutRecognized = true;
	}
	else
	{
		DetectedLayout.Suffix = FNTexturePackerUtils::ComposeSuffixForRoles(*Settings, Roles);
		DetectedLayout.Compression = TC_Masks;
	}

	if (!bOutputNameEdited)
	{
		const FString BaseName = FNTexturePackerUtils::CommonBaseName(BaseNames);
		OutputName = FNTexturePackerUtils::ComposeAssetName(BaseName, DetectedLayout.Suffix);
	}

	if (!bOutputPathEdited && FirstTexture != nullptr)
	{
		OutputPath = FNTexturePackerUtils::GetPackagePath(FirstTexture);
	}
}

FString SNTexturePackerPackPanel::GetSlotObjectPath(const int32 ChannelIndex) const
{
	const UTexture2D* Texture = Slots[ChannelIndex].Texture.Get();
	return Texture != nullptr ? Texture->GetPathName() : FString();
}

void SNTexturePackerPackPanel::HandleSlotObjectChanged(const FAssetData& AssetData, const int32 ChannelIndex)
{
	Slots[ChannelIndex].Texture = Cast<UTexture2D>(AssetData.GetAsset());

	// Clearing a slot leaves its source choice behind, which would read as a texture channel selection on a
	// slot that has no texture. Back to Auto, which is what a freshly assigned one wants anyway.
	if (!Slots[ChannelIndex].Texture.IsValid())
	{
		Slots[ChannelIndex].Source = ENTextureChannelSource::Auto;
		Slots[ChannelIndex].bInvert = false;
	}

	RefreshDetection();
}

FText SNTexturePackerPackPanel::GetSourceText(const int32 ChannelIndex) const
{
	return NEXUS::ToolingEditor::TexturePacker::UI::GetSourceLabel(Slots[ChannelIndex].Source);
}

ECheckBoxState SNTexturePackerPackPanel::GetInvertState(const int32 ChannelIndex) const
{
	return Slots[ChannelIndex].bInvert ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerPackPanel::HandleInvertChanged(const ECheckBoxState NewState, const int32 ChannelIndex)
{
	Slots[ChannelIndex].bInvert = NewState == ECheckBoxState::Checked;
}

ECheckBoxState SNTexturePackerPackPanel::GetTreatAsLinearState(const int32 ChannelIndex) const
{
	return Slots[ChannelIndex].bTreatAsLinear ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerPackPanel::HandleTreatAsLinearChanged(const ECheckBoxState NewState, const int32 ChannelIndex)
{
	Slots[ChannelIndex].bTreatAsLinear = NewState == ECheckBoxState::Checked;
}

float SNTexturePackerPackPanel::GetConstantValue(const int32 ChannelIndex) const
{
	return Slots[ChannelIndex].ConstantValue;
}

void SNTexturePackerPackPanel::HandleConstantValueChanged(const float NewValue, const int32 ChannelIndex)
{
	Slots[ChannelIndex].ConstantValue = NewValue;
}

EVisibility SNTexturePackerPackPanel::GetConstantVisibility(const int32 ChannelIndex) const
{
	return Slots[ChannelIndex].Texture.IsValid() ? EVisibility::Collapsed : EVisibility::Visible;
}

bool SNTexturePackerPackPanel::IsSlotTextureAssigned(const int32 ChannelIndex) const
{
	return Slots[ChannelIndex].Texture.IsValid();
}

void SNTexturePackerPackPanel::HandleOutputPathChanged(const FText& NewText)
{
	OutputPath = NewText.ToString();
	bOutputPathEdited = true;
}

void SNTexturePackerPackPanel::HandleOutputNameChanged(const FText& NewText)
{
	OutputName = NewText.ToString();
	bOutputNameEdited = true;
}

FReply SNTexturePackerPackPanel::HandleUseSelectedPathClicked()
{
	const FString Selected = FNTexturePackerUtils::GetContentBrowserPath();
	if (!Selected.IsEmpty())
	{
		OutputPath = Selected;
		bOutputPathEdited = true;
	}

	return FReply::Handled();
}

FReply SNTexturePackerPackPanel::HandleResetNamingClicked()
{
	bOutputNameEdited = false;
	bOutputPathEdited = false;
	RefreshDetection();
	return FReply::Handled();
}

ECheckBoxState SNTexturePackerPackPanel::GetSixteenBitState() const
{
	return bSixteenBit ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerPackPanel::HandleSixteenBitChanged(const ECheckBoxState NewState)
{
	bSixteenBit = NewState == ECheckBoxState::Checked;
}

ECheckBoxState SNTexturePackerPackPanel::GetOverwriteState() const
{
	return bOverwriteExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerPackPanel::HandleOverwriteChanged(const ECheckBoxState NewState)
{
	bOverwriteExisting = NewState == ECheckBoxState::Checked;
}

FText SNTexturePackerPackPanel::GetDetectionText() const
{
	if (DetectedLayout.Suffix.IsEmpty())
	{
		return LOCTEXT("Pack_Detected_None",
			"Nothing assigned yet, or nothing whose name says what it holds.");
	}

	if (!bLayoutRecognized)
	{
		return FText::Format(LOCTEXT("Pack_Detected_Composed",
			"No known layout covers this arrangement. The name was composed as {0}."),
			FText::FromString(DetectedLayout.Suffix));
	}

	const FText Standard = DetectedLayout.bIndustryStandard
		? LOCTEXT("Pack_Detected_Standard", " (industry standard)")
		: FText::GetEmpty();

	return FText::Format(LOCTEXT("Pack_Detected", "Detected {0} - {1}{2}"),
		FText::FromString(DetectedLayout.Suffix), DetectedLayout.GetDisplayText(), Standard);
}

FSlateColor SNTexturePackerPackPanel::GetDetectionColor() const
{
	return bLayoutRecognized ? FSlateColor::UseForeground() : FSlateColor::UseSubduedForeground();
}

FText SNTexturePackerPackPanel::GetMessageText() const
{
	if (Messages.Num() == 0) return FText::GetEmpty();

	TArray<FString> Lines;
	Lines.Reserve(Messages.Num());
	for (const FText& Message : Messages)
	{
		Lines.Add(Message.ToString());
	}

	return FText::FromString(FString::Join(Lines, TEXT("\n")));
}

EVisibility SNTexturePackerPackPanel::GetMessageVisibility() const
{
	return Messages.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SNTexturePackerPackPanel::GetMessageColor() const
{
	return bLastRunFailed ? FSlateColor(FLinearColor(1.0f, 0.4f, 0.4f)) : FSlateColor::UseSubduedForeground();
}

bool SNTexturePackerPackPanel::CanPack() const
{
	if (OutputPath.IsEmpty() || OutputName.IsEmpty()) return false;

	for (const FSlotState& Slot : Slots)
	{
		if (Slot.Texture.IsValid()) return true;
	}

	return false;
}

FReply SNTexturePackerPackPanel::HandlePackClicked()
{
	FNTexturePackRequest Request;
	Request.Slots.SetNum(4);

	for (int32 ChannelIndex = 0; ChannelIndex < 4; ChannelIndex++)
	{
		const FSlotState& State = Slots[ChannelIndex];
		FNTexturePackSlot& Slot = Request.Slots[ChannelIndex];

		Slot.Texture = State.Texture.Get();
		Slot.Source = State.Texture.IsValid() ? State.Source : ENTextureChannelSource::Constant;
		Slot.bInvert = State.bInvert;
		Slot.bTreatSourceAsLinear = State.bTreatAsLinear;
		Slot.ConstantValue = State.ConstantValue;
	}

	Request.PackagePath = OutputPath;
	Request.AssetName = OutputName;
	Request.bSixteenBit = bSixteenBit;
	Request.bOverwriteExisting = bOverwriteExisting;
	Request.Compression = DetectedLayout.Compression;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	Request.bCheckOut = Settings->bCheckOutAssets;

	FNTexturePackerResult Result;
	FNTexturePackerJob::Pack(Request, Result);

	Messages = Result.Warnings;
	bLastRunFailed = !Result.bSucceeded;

	if (!Result.bSucceeded)
	{
		Messages.Add(Result.Error);
		return FReply::Handled();
	}

	Messages.Insert(FText::Format(LOCTEXT("Pack_Done",
		"Wrote {0}. It is unsaved - review it, then save."), FText::FromString(OutputName)), 0);

	if (Settings->bSelectResults)
	{
		TArray<UObject*> Written;
		for (const TObjectPtr<UTexture2D>& Texture : Result.Textures)
		{
			Written.Add(Texture);
		}
		FNTexturePackerUtils::SelectAssetsInContentBrowser(Written);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
