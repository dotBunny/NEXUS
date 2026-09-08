// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/SNTexturePackerUnpackPanel.h"

#include "AssetThumbnail.h"
#include "ImageCore.h"
#include "PropertyCustomizationHelpers.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Texture2D.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "TexturePacker/NTexturePackerUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker::UnpackUI
{
	/** One picker's worth of thumbnail, plus headroom. */
	constexpr int32 ThumbnailPoolSize = 8;

	/** @return The channels an output covers, as the letters that name them ("RGB"). */
	FString GetChannelLetters(const ENTextureChannel FirstChannel, const int32 Length)
	{
		static const TCHAR* Letters[] = { TEXT("R"), TEXT("G"), TEXT("B"), TEXT("A") };

		const int32 First = FMath::Clamp(static_cast<int32>(FirstChannel), 0, 3);
		const int32 Count = FMath::Clamp(Length, 1, 4 - First);

		FString Result;
		for (int32 Offset = 0; Offset < Count; Offset++)
		{
			Result += Letters[First + Offset];
		}

		return Result;
	}

	/** @return A layout describing four channels nobody has identified, for the manual case. */
	FNTexturePackedLayout MakeCustomLayout()
	{
		FNTexturePackedLayout Custom;
		Custom.Suffix = TEXT("Custom");
		Custom.DisplayName = TEXT("One texture per channel");
		return Custom;
	}
}

void SNTexturePackerUnpackPanel::Construct(const FArguments& InArgs)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UnpackUI;

	ThumbnailPool = MakeShared<FAssetThumbnailPool>(ThumbnailPoolSize);

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	bSixteenBit = Settings->bSixteenBitOutput;

	OutputPath = FNTexturePackerUtils::GetContentBrowserPath();

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
				.Text(LOCTEXT("Unpack_Explanation",
					"Pick a packed texture and it is read for what its name says it holds. Channels carrying "
					"the same map come out as one texture. A channel holding a single value throughout is "
					"left switched off, because it is not a map."))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Unpack_Source", "Texture"))
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
				.ObjectPath(this, &SNTexturePackerUnpackPanel::GetSourceObjectPath)
				.OnObjectChanged(this, &SNTexturePackerUnpackPanel::HandleSourceObjectChanged)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SNTexturePackerUnpackPanel::GetDetectionText)
				.ColorAndOpacity(this, &SNTexturePackerUnpackPanel::GetDetectionColor)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SComboButton)
				.OnGetMenuContent(this, &SNTexturePackerUnpackPanel::BuildLayoutMenu)
				.ToolTipText(LOCTEXT("Unpack_Layout_Tooltip",
					"Read the texture as a different arrangement. Worth reaching for where the name says "
					"nothing, or says the wrong thing."))
				.ButtonContent()
				[
					SNew(STextBlock).Text(LOCTEXT("Unpack_Layout", "Layout"))
				]
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 4.0f)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			[
				SAssignNew(OutputRows, SVerticalBox)
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
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Unpack_Path", "Path"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Text(this, &SNTexturePackerUnpackPanel::GetOutputPathText)
				.OnTextChanged(this, &SNTexturePackerUnpackPanel::HandleOutputPathChanged)
				.HintText(LOCTEXT("Unpack_Path_Hint", "/Game/..."))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Unpack_UsePath", "Use Selected"))
				.ToolTipText(LOCTEXT("Unpack_UsePath_Tooltip", "Take the folder currently selected in the Content Browser."))
				.OnClicked(this, &SNTexturePackerUnpackPanel::HandleUseSelectedPathClicked)
			]
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
				.IsChecked(this, &SNTexturePackerUnpackPanel::GetSixteenBitState)
				.OnCheckStateChanged(this, &SNTexturePackerUnpackPanel::HandleSixteenBitChanged)
				.ToolTipText(LOCTEXT("Unpack_SixteenBit_Tooltip",
					"Write 16 bits per channel. A gamma-encoded output is written at 8 either way."))
				[
					SNew(STextBlock).Text(LOCTEXT("Unpack_SixteenBit", "16-bit output"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 12.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SNTexturePackerUnpackPanel::GetOverwriteState)
				.OnCheckStateChanged(this, &SNTexturePackerUnpackPanel::HandleOverwriteChanged)
				.ToolTipText(LOCTEXT("Unpack_Overwrite_Tooltip",
					"Replace assets already standing at the output paths, checking them out first."))
				[
					SNew(STextBlock).Text(LOCTEXT("Unpack_Overwrite", "Overwrite existing"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(this, &SNTexturePackerUnpackPanel::GetTreatAsLinearState)
				.OnCheckStateChanged(this, &SNTexturePackerUnpackPanel::HandleTreatAsLinearChanged)
				.ToolTipText(LOCTEXT("Unpack_Raw_Tooltip",
					"Take the source's stored values as they are, ignoring its sRGB flag."))
				[
					SNew(STextBlock).Text(LOCTEXT("Unpack_Raw", "Raw source"))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(this, &SNTexturePackerUnpackPanel::GetMessageText)
			.Visibility(this, &SNTexturePackerUnpackPanel::GetMessageVisibility)
			.ColorAndOpacity(this, &SNTexturePackerUnpackPanel::GetMessageColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(8.0f, 4.0f, 8.0f, 8.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Unpack_Run", "Unpack"))
			.ToolTipText(LOCTEXT("Unpack_Run_Tooltip", "Write the textures. They are left unsaved for review."))
			.IsEnabled(this, &SNTexturePackerUnpackPanel::CanUnpack)
			.OnClicked(this, &SNTexturePackerUnpackPanel::HandleUnpackClicked)
		]
	];

	RebuildOutputRows();
}

void SNTexturePackerUnpackPanel::SetSourceTexture(UTexture2D* Texture)
{
	SourceTexture = Texture;
	bOutputPathEdited = false;
	Messages.Reset();
	bLastRunFailed = false;
	RefreshDetection();
}

void SNTexturePackerUnpackPanel::RefreshDetection()
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UnpackUI;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	const UTexture2D* Texture = SourceTexture.Get();

	if (Texture == nullptr)
	{
		BaseName.Reset();
		Layout = FNTexturePackedLayout();
		bLayoutRecognized = false;
		Outputs.Reset();
		RebuildOutputRows();
		return;
	}

	if (!bOutputPathEdited)
	{
		OutputPath = FNTexturePackerUtils::GetPackagePath(Texture);
	}

	FNTexturePackedLayout Detected;
	if (FNTexturePackerUtils::DetectLayout(*Settings, Texture->GetName(), Detected, BaseName))
	{
		ApplyLayout(Detected, true);
		return;
	}

	// Nothing named it, so every channel is offered separately rather than guessed at - which is the state
	// somebody would build by hand anyway, one role picker at a time.
	ApplyLayout(MakeCustomLayout(), false);
}

void SNTexturePackerUnpackPanel::ApplyLayout(const FNTexturePackedLayout& InLayout, const bool bRecognized)
{
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	Layout = InLayout;
	bLayoutRecognized = bRecognized;
	Outputs.Reset();

	const TArray<FName> Roles = InLayout.GetRoles();

	bool bAnyRole = false;
	for (const FName& Role : Roles)
	{
		if (!Role.IsNone())
		{
			bAnyRole = true;
			break;
		}
	}

	if (bAnyRole)
	{
		for (const FNTextureChannelRun& Run : FNTexturePackerUtils::BuildChannelRuns(Roles))
		{
			FOutputState State;
			State.Output.RoleId = Run.RoleId;
			State.Output.FirstChannel = Run.FirstChannel;
			State.Output.Length = Run.Length;
			FNTexturePackerJob::ApplyRoleToOutput(*Settings, BaseName, State.Output);
			Outputs.Add(State);
		}
	}
	else
	{
		// The custom layout: four rows, one per channel, each waiting to be told what it is.
		for (int32 ChannelIndex = 0; ChannelIndex < 4; ChannelIndex++)
		{
			FOutputState State;
			State.Output.FirstChannel = static_cast<ENTextureChannel>(ChannelIndex);
			State.Output.Length = 1;
			FNTexturePackerJob::ApplyRoleToOutput(*Settings, BaseName, State.Output);
			Outputs.Add(State);
		}
	}

	DisableUniformOutputs();
	RebuildOutputRows();
}

void SNTexturePackerUnpackPanel::DisableUniformOutputs()
{
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	if (!Settings->bSkipUniformChannels) return;

	UTexture2D* Texture = SourceTexture.Get();
	if (Texture == nullptr) return;

	FImage Packed;
	FText ReadError;
	if (!FNTexturePackerUtils::ReadTextureLinear(Texture, Packed, bTreatAsLinear, ReadError)) return;

	for (FOutputState& State : Outputs)
	{
		bool bUniform = true;
		for (int32 Offset = 0; Offset < State.Output.Length && bUniform; Offset++)
		{
			const int32 ChannelIndex = static_cast<int32>(State.Output.FirstChannel) + Offset;
			if (ChannelIndex > 3) break;

			bUniform = FNTexturePackerUtils::IsChannelUniform(Packed,
				static_cast<ENTextureChannel>(ChannelIndex));
		}

		if (bUniform)
		{
			State.Output.bEnabled = false;
		}
	}
}

void SNTexturePackerUnpackPanel::RebuildOutputRows()
{
	if (!OutputRows.IsValid()) return;

	OutputRows->ClearChildren();

	if (Outputs.Num() == 0)
	{
		OutputRows->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Unpack_Empty", "Pick a texture to see what it holds."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			];
		return;
	}

	for (int32 OutputIndex = 0; OutputIndex < Outputs.Num(); OutputIndex++)
	{
		OutputRows->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				BuildOutputRow(OutputIndex)
			];
	}
}

TSharedRef<SWidget> SNTexturePackerUnpackPanel::BuildOutputRow(const int32 OutputIndex)
{
	return SNew(SBorder)
		.Padding(6.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SNTexturePackerUnpackPanel::GetOutputEnabledState, OutputIndex)
				.OnCheckStateChanged(this, &SNTexturePackerUnpackPanel::HandleOutputEnabledChanged, OutputIndex)
				.ToolTipText(LOCTEXT("Unpack_Enabled_Tooltip", "Write this map out."))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(34.0f)
				[
					SNew(STextBlock)
					.Text(this, &SNTexturePackerUnpackPanel::GetOutputChannelText, OutputIndex)
					.Justification(ETextJustify::Center)
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(150.0f)
				[
					SNew(SComboButton)
					.OnGetMenuContent(this, &SNTexturePackerUnpackPanel::BuildRoleMenu, OutputIndex)
					.ToolTipText(LOCTEXT("Unpack_Role_Tooltip",
						"What these channels hold. It decides the suffix, the gamma and the compression the "
						"generated texture gets."))
					.ButtonContent()
					[
						SNew(STextBlock).Text(this, &SNTexturePackerUnpackPanel::GetOutputRoleText, OutputIndex)
					]
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SEditableTextBox)
				.Text(this, &SNTexturePackerUnpackPanel::GetOutputNameText, OutputIndex)
				.OnTextChanged(this, &SNTexturePackerUnpackPanel::HandleOutputNameChanged, OutputIndex)
			]
		];
}

TSharedRef<SWidget> SNTexturePackerUnpackPanel::BuildLayoutMenu()
{
	using namespace NEXUS::ToolingEditor::TexturePacker::UnpackUI;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(TEXT("Standard"), LOCTEXT("Unpack_Menu_Standard", "Industry Standard"));
	for (const FNTexturePackedLayout& Candidate : Settings->Layouts)
	{
		if (!Candidate.bIndustryStandard) continue;

		MenuBuilder.AddMenuEntry(
			FText::Format(LOCTEXT("Unpack_Menu_Layout", "{0} - {1}"),
				FText::FromString(Candidate.Suffix), Candidate.GetDisplayText()),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Candidate]() { ApplyLayout(Candidate, true); })),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(TEXT("Project"), LOCTEXT("Unpack_Menu_Project", "Project"));
	for (const FNTexturePackedLayout& Candidate : Settings->Layouts)
	{
		if (Candidate.bIndustryStandard) continue;

		MenuBuilder.AddMenuEntry(
			FText::Format(LOCTEXT("Unpack_Menu_Layout", "{0} - {1}"),
				FText::FromString(Candidate.Suffix), Candidate.GetDisplayText()),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Candidate]() { ApplyLayout(Candidate, true); })),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(TEXT("Other"), LOCTEXT("Unpack_Menu_Other", "Other"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Unpack_Menu_Custom", "Custom - one texture per channel"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]() { ApplyLayout(MakeCustomLayout(), false); })),
		NAME_None,
		EUserInterfaceActionType::Button);
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SNTexturePackerUnpackPanel::BuildRoleMenu(const int32 OutputIndex)
{
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(TEXT("Standard"), LOCTEXT("Unpack_Menu_Standard", "Industry Standard"));
	for (const FNTextureChannelRole& Role : Settings->Roles)
	{
		if (!Role.bIndustryStandard) continue;

		const FName RoleId = Role.Id;
		MenuBuilder.AddMenuEntry(
			FText::Format(LOCTEXT("Unpack_Menu_Role", "{0} (_{1})"),
				Role.GetDisplayText(), FText::FromString(Role.GetCanonicalSuffix())),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, OutputIndex, RoleId]()
				{
					if (!Outputs.IsValidIndex(OutputIndex)) return;

					Outputs[OutputIndex].Output.RoleId = RoleId;

					// A hand-typed name survives a role change; a proposed one follows it.
					const FString PreviousName = Outputs[OutputIndex].Output.AssetName;
					FNTexturePackerJob::ApplyRoleToOutput(*UNTexturePackerSettings::Get(), BaseName,
						Outputs[OutputIndex].Output);
					if (Outputs[OutputIndex].bNameEdited)
					{
						Outputs[OutputIndex].Output.AssetName = PreviousName;
					}
				})),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(TEXT("Project"), LOCTEXT("Unpack_Menu_Project", "Project"));
	for (const FNTextureChannelRole& Role : Settings->Roles)
	{
		if (Role.bIndustryStandard) continue;

		const FName RoleId = Role.Id;
		MenuBuilder.AddMenuEntry(
			FText::Format(LOCTEXT("Unpack_Menu_Role", "{0} (_{1})"),
				Role.GetDisplayText(), FText::FromString(Role.GetCanonicalSuffix())),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, OutputIndex, RoleId]()
				{
					if (!Outputs.IsValidIndex(OutputIndex)) return;

					Outputs[OutputIndex].Output.RoleId = RoleId;

					const FString PreviousName = Outputs[OutputIndex].Output.AssetName;
					FNTexturePackerJob::ApplyRoleToOutput(*UNTexturePackerSettings::Get(), BaseName,
						Outputs[OutputIndex].Output);
					if (Outputs[OutputIndex].bNameEdited)
					{
						Outputs[OutputIndex].Output.AssetName = PreviousName;
					}
				})),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

FString SNTexturePackerUnpackPanel::GetSourceObjectPath() const
{
	const UTexture2D* Texture = SourceTexture.Get();
	return Texture != nullptr ? Texture->GetPathName() : FString();
}

void SNTexturePackerUnpackPanel::HandleSourceObjectChanged(const FAssetData& AssetData)
{
	SetSourceTexture(Cast<UTexture2D>(AssetData.GetAsset()));
}

ECheckBoxState SNTexturePackerUnpackPanel::GetOutputEnabledState(const int32 OutputIndex) const
{
	if (!Outputs.IsValidIndex(OutputIndex)) return ECheckBoxState::Unchecked;

	return Outputs[OutputIndex].Output.bEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerUnpackPanel::HandleOutputEnabledChanged(const ECheckBoxState NewState, const int32 OutputIndex)
{
	if (!Outputs.IsValidIndex(OutputIndex)) return;

	Outputs[OutputIndex].Output.bEnabled = NewState == ECheckBoxState::Checked;
}

FText SNTexturePackerUnpackPanel::GetOutputRoleText(const int32 OutputIndex) const
{
	if (!Outputs.IsValidIndex(OutputIndex)) return FText::GetEmpty();

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();
	if (const FNTextureChannelRole* Role = Settings->FindRole(Outputs[OutputIndex].Output.RoleId))
	{
		return Role->GetDisplayText();
	}

	return LOCTEXT("Unpack_Role_None", "Unassigned");
}

FText SNTexturePackerUnpackPanel::GetOutputChannelText(const int32 OutputIndex) const
{
	if (!Outputs.IsValidIndex(OutputIndex)) return FText::GetEmpty();

	return FText::FromString(NEXUS::ToolingEditor::TexturePacker::UnpackUI::GetChannelLetters(
		Outputs[OutputIndex].Output.FirstChannel, Outputs[OutputIndex].Output.Length));
}

FText SNTexturePackerUnpackPanel::GetOutputNameText(const int32 OutputIndex) const
{
	if (!Outputs.IsValidIndex(OutputIndex)) return FText::GetEmpty();

	return FText::FromString(Outputs[OutputIndex].Output.AssetName);
}

void SNTexturePackerUnpackPanel::HandleOutputNameChanged(const FText& NewText, const int32 OutputIndex)
{
	if (!Outputs.IsValidIndex(OutputIndex)) return;

	Outputs[OutputIndex].Output.AssetName = NewText.ToString();
	Outputs[OutputIndex].bNameEdited = true;
}

void SNTexturePackerUnpackPanel::HandleOutputPathChanged(const FText& NewText)
{
	OutputPath = NewText.ToString();
	bOutputPathEdited = true;
}

FReply SNTexturePackerUnpackPanel::HandleUseSelectedPathClicked()
{
	const FString Selected = FNTexturePackerUtils::GetContentBrowserPath();
	if (!Selected.IsEmpty())
	{
		OutputPath = Selected;
		bOutputPathEdited = true;
	}

	return FReply::Handled();
}

ECheckBoxState SNTexturePackerUnpackPanel::GetSixteenBitState() const
{
	return bSixteenBit ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerUnpackPanel::HandleSixteenBitChanged(const ECheckBoxState NewState)
{
	bSixteenBit = NewState == ECheckBoxState::Checked;
}

ECheckBoxState SNTexturePackerUnpackPanel::GetOverwriteState() const
{
	return bOverwriteExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerUnpackPanel::HandleOverwriteChanged(const ECheckBoxState NewState)
{
	bOverwriteExisting = NewState == ECheckBoxState::Checked;
}

ECheckBoxState SNTexturePackerUnpackPanel::GetTreatAsLinearState() const
{
	return bTreatAsLinear ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SNTexturePackerUnpackPanel::HandleTreatAsLinearChanged(const ECheckBoxState NewState)
{
	bTreatAsLinear = NewState == ECheckBoxState::Checked;
}

FText SNTexturePackerUnpackPanel::GetDetectionText() const
{
	if (!SourceTexture.IsValid())
	{
		return LOCTEXT("Unpack_Detected_None", "No texture selected.");
	}

	if (!bLayoutRecognized)
	{
		return LOCTEXT("Unpack_Detected_Unknown",
			"Nothing in the name says what this holds. Pick a layout, or set each channel by hand.");
	}

	const FText Standard = Layout.bIndustryStandard
		? LOCTEXT("Unpack_Detected_Standard", " (industry standard)")
		: FText::GetEmpty();

	return FText::Format(LOCTEXT("Unpack_Detected", "Reading as {0} - {1}{2}"),
		FText::FromString(Layout.Suffix), Layout.GetDisplayText(), Standard);
}

FSlateColor SNTexturePackerUnpackPanel::GetDetectionColor() const
{
	return bLayoutRecognized ? FSlateColor::UseForeground() : FSlateColor::UseSubduedForeground();
}

FText SNTexturePackerUnpackPanel::GetMessageText() const
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

EVisibility SNTexturePackerUnpackPanel::GetMessageVisibility() const
{
	return Messages.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SNTexturePackerUnpackPanel::GetMessageColor() const
{
	return bLastRunFailed ? FSlateColor(FLinearColor(1.0f, 0.4f, 0.4f)) : FSlateColor::UseSubduedForeground();
}

bool SNTexturePackerUnpackPanel::CanUnpack() const
{
	if (!SourceTexture.IsValid() || OutputPath.IsEmpty()) return false;

	for (const FOutputState& State : Outputs)
	{
		if (State.Output.bEnabled && !State.Output.AssetName.IsEmpty()) return true;
	}

	return false;
}

FReply SNTexturePackerUnpackPanel::HandleUnpackClicked()
{
	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FNTextureUnpackRequest Request;
	Request.Texture = SourceTexture.Get();
	Request.PackagePath = OutputPath;
	Request.bSixteenBit = bSixteenBit;
	Request.bOverwriteExisting = bOverwriteExisting;
	Request.bTreatSourceAsLinear = bTreatAsLinear;
	Request.bCheckOut = Settings->bCheckOutAssets;

	for (const FOutputState& State : Outputs)
	{
		Request.Outputs.Add(State.Output);
	}

	FNTexturePackerResult Result;
	FNTexturePackerJob::Unpack(Request, Result);

	Messages = Result.Warnings;
	bLastRunFailed = !Result.bSucceeded;

	if (!Result.bSucceeded)
	{
		Messages.Add(Result.Error);
		return FReply::Handled();
	}

	Messages.Insert(FText::Format(LOCTEXT("Unpack_Done",
		"Wrote {0} texture(s). They are unsaved - review them, then save."), Result.Textures.Num()), 0);

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
