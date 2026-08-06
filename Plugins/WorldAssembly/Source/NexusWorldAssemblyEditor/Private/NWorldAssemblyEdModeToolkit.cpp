// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdModeToolkit.h"

#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "Rails/NWorldAssemblyEdModeCellRail.h"
#include "Rails/NWorldAssemblyEdModeJunctionRail.h"
#include "Rails/NWorldAssemblyEdModeOrganRail.h"
#include "Rails/NWorldAssemblyEdModeWorldRail.h"
#include "Styling/AppStyle.h"
#include "Tools/UEdMode.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

FNWorldAssemblyEdModeToolkitBuilder::FNWorldAssemblyEdModeToolkitBuilder(FToolkitBuilderArgs& Args)
	: FToolkitBuilder(Args)
{
}

void FNWorldAssemblyEdModeToolkitBuilder::AddCategory(const TSharedPtr<FToolPalette>& Palette, const TAttribute<bool>& Predicate)
{
	if (!Palette.IsValid() || !Palette->LoadToolPaletteAction.IsValid()) return;

	AddPalette(Palette);

	if (!Predicate.IsSet() || !LoadToolPaletteCommandList.IsValid()) return;

	const TSharedRef<FUICommandInfo> LoadAction = Palette->LoadToolPaletteAction.ToSharedRef();
	const FUIAction* Existing = LoadToolPaletteCommandList->GetActionForCommand(LoadAction);
	if (Existing == nullptr) return;

	// Copied rather than rebuilt: the Execute the base bound calls a private member, so carrying its action across
	// unchanged and swapping in the predicate is the only way to keep the button working.
	FUIAction Updated = *Existing;
	Updated.CanExecuteAction = FCanExecuteAction::CreateLambda([Predicate]()
	{
		return Predicate.Get();
	});

	LoadToolPaletteCommandList->UnmapAction(LoadAction);
	LoadToolPaletteCommandList->MapAction(LoadAction, Updated);
}

void FNWorldAssemblyEdModeToolkitBuilder::UpdateContentForCategory(FName InActiveCategoryName, FText InActiveCategoryText)
{
	if (!MainContentVerticalBox.IsValid()) return;

	// One scroll box holding the whole content column, in the only slot that fills — AutoHeight would let the box take
	// its content's height and never scroll. The category rail is not in here: it is a sibling of this column in the
	// widget the base assembles, so it stays fixed while the panel beside it scrolls.
	const TSharedRef<SVerticalBox> ScrolledContent = SNew(SVerticalBox);

	MainContentVerticalBox->AddSlot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ScrolledContent
			]
		];

	// Context that applies across a category — the pickers naming what the buttons below act on.
	if (ToolkitSections->Header)
	{
		ScrolledContent->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				ToolkitSections->Header->AsShared()
			];
	}

	// Category content, in the palette's own position. Two sources, because a category can use either: the palette
	// for the flat toolbar FToolPalette can express, and this for the titled groups it cannot. A category registered
	// with an empty palette contributes nothing from the first, and vice versa.
	if (CategoryContent.IsValid())
	{
		ScrolledContent->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				CategoryContent.ToSharedRef()
			];
	}

	ScrolledContent->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		[
			GetToolPaletteWidget()
		];

	// The active tool's properties scroll with the rest rather than competing for height: a second filling slot
	// would split the panel evenly with the scroll box above, reserving half of it even with no tool running.
	if (ToolkitSections->DetailsView)
	{
		ScrolledContent->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				ToolkitSections->DetailsView->AsShared()
			];
	}
}

void FNWorldAssemblyEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	// Opt into FToolkitBuilder before Super::Init, which reads the flag while deciding what tabs to request: with the
	// builder on, the palettes live inside the mode panel rather than the separate vertical-toolbar tab.
	bUsesToolkitBuilder = true;

	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	RegisterPalettes();

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Fill)
		.Padding(0)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// The builder scrolls its own content column now (see FNWorldAssemblyEdModeToolkitBuilder), which is what
			// keeps the category rail fixed while the panel beside it scrolls.
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				ToolkitBuilder->GenerateWidget()->AsShared()
			]

			// Warnings sit outside that scroll region, so they stay on screen no matter how far the content above is
			// scrolled — which for something reporting live cell state is the point of it.
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateWarningFooter()
			]
		];
}

FText FNWorldAssemblyEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("NWorldAssemblyEdModeToolkit_Name", "World Assembly");
}

bool FNWorldAssemblyEdModeToolkit::IsPaletteActive(const TSharedPtr<FUICommandInfo>& PaletteCommand) const
{
	if (!ToolkitBuilder.IsValid() || !PaletteCommand.IsValid()) return false;

	return ToolkitBuilder->GetActivePaletteName() == PaletteCommand->GetCommandName();
}

TSharedRef<SWidget> FNWorldAssemblyEdModeToolkit::CreateWarningFooter()
{
	// The builder's footer slot carries no padding of its own, so the box would otherwise sit flush against the panel
	// edges. Visibility goes on this outer box rather than the border: with it on the border, the padding below would
	// survive the collapse and leave a gap at the bottom of every panel with nothing to warn about.
	return SNew(SBox)
		.Padding(FMargin(6.0f, 6.0f, 6.0f, 6.0f))
		.Visibility_Lambda([]()
		{
			return UNWorldAssemblyEdMode::GetWarningText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
		})
		[
			SNew(SBorder)
			// The amber-outlined box rather than the red one: everything GetWarningText reports is a "this will not
			// regenerate on save" advisory, not a failure.
			.BorderImage(FAppStyle::Get().GetBrush("RoundedWarning"))
			.Padding(FMargin(8.0f, 6.0f))
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FStyleColors::Warning)
				.Text_Static(&UNWorldAssemblyEdMode::GetWarningText)
			]
		];
}




#undef LOCTEXT_NAMESPACE

TSharedRef<SWidget> FNWorldAssemblyEdModeToolkit::GateOnCategory(const TSharedRef<SWidget>& Widget, const TSharedPtr<FUICommandInfo>& CategoryCommand) const
{
	return SNew(SBox)
		.Visibility_Lambda([this, CategoryCommand]()
		{
			return IsPaletteActive(CategoryCommand) ? EVisibility::Visible : EVisibility::Collapsed;
		})
		[
			Widget
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeToolkit::CreateContextHeader() const
{
	const TSharedRef<SVerticalBox> Header = SNew(SVerticalBox);

	for (const TSharedRef<FNWorldAssemblyEdModeRail>& Rail : Rails)
	{
		const TSharedPtr<SWidget> RailHeader = Rail->CreateHeader();
		if (!RailHeader.IsValid()) continue;

		Header->AddSlot()
			.AutoHeight()
			.Padding(4.0f, 8.0f, 4.0f, 2.0f)
			[
				GateOnCategory(RailHeader.ToSharedRef(), Rail->GetCategoryCommand())
			];
	}

	return Header;
}

TSharedRef<SWidget> FNWorldAssemblyEdModeToolkit::CreateCategoryContent() const
{
	const TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	for (const TSharedRef<FNWorldAssemblyEdModeRail>& Rail : Rails)
	{
		const TSharedPtr<SWidget> RailContent = Rail->CreateContent();
		if (!RailContent.IsValid()) continue;

		Content->AddSlot()
			.AutoHeight()
			[
				GateOnCategory(RailContent.ToSharedRef(), Rail->GetCategoryCommand())
			];
	}

	return Content;
}

void FNWorldAssemblyEdModeToolkit::RegisterPalettes()
{
	const FNWorldAssemblyEditorCommands& Commands = FNWorldAssemblyEditorCommands::Get();

	ToolkitSections = MakeShared<FToolkitSections>();
	ToolkitSections->DetailsView = ModeDetailsView;

	// The rail buttons resolve their actions against the toolkit's own command list, so fold in the per-area lists the
	// commands were already mapped into. This is also what scopes them to the mode: the Organ bindings used to be
	// appended to the level editor's global action list, so their chords fired whether or not the mode was open.
	const TSharedRef<FUICommandList> ToolkitCommandList = GetToolkitCommands();
	ToolkitCommandList->Append(Commands.CommandList_Cell.ToSharedRef());
	ToolkitCommandList->Append(Commands.CommandList_CellJunction.ToSharedRef());
	ToolkitCommandList->Append(Commands.CommandList_World.ToSharedRef());
	ToolkitCommandList->Append(Commands.CommandList_Organ.ToSharedRef());

	// Order here is the order of the buttons on the rail.
	Rails.Add(MakeShared<FNWorldAssemblyEdModeWorldRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeCellRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeJunctionRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeOrganRail>(ToolkitCommandList));

	// Warnings are deliberately not handed to a FToolkitSections slot. ModeWarningArea sits above the palette, where a
	// warning appearing shifts every button down under the cursor, and is typed as a bare STextBlock that cannot carry
	// a border; Footer is inside the builder's output, which Init puts in a scroll box, so it would scroll away. Init
	// pins it below that scroll region instead.
	ToolkitSections->Header = CreateContextHeader();

	FToolkitBuilderArgs ToolkitBuilderArgs(GetScriptableEditorMode()->GetModeInfo().ToolbarCustomizationName);
	ToolkitBuilderArgs.ToolkitCommandList = ToolkitCommandList;
	ToolkitBuilderArgs.ToolkitSections = ToolkitSections;
	// Collapsed because the header draws the category title itself, at the top where it belongs. The builder's own copy
	// renders inside the palette widget, below whatever the header contains.
	ToolkitBuilderArgs.SelectedCategoryTitleVisibility = EVisibility::Collapsed;

	const TSharedRef<FNWorldAssemblyEdModeToolkitBuilder> Builder = MakeShared<FNWorldAssemblyEdModeToolkitBuilder>(ToolkitBuilderArgs);
	ToolkitBuilder = Builder;

	// Leaving a category leaves the tool that belongs to it. The tools are scoped to their category — the hull tools
	// drive the Cell rail's overlay, junction placement the Junction rail's — so one left running under a category
	// that does not present it keeps drawing handles and swallowing viewport clicks with nothing on screen to say
	// which tool is doing it or how to stop it. Safe from inside the broadcast: UEditorInteractiveToolsContext defers
	// the shutdown to its next tick rather than tearing the tool down under the builder that is mid-switch.
	Builder->OnActivePaletteChanged.AddStatic(&UNWorldAssemblyEdMode::EndActiveTool);

	for (const TSharedRef<FNWorldAssemblyEdModeRail>& Rail : Rails)
	{
		Builder->AddCategory(
			MakeShared<FToolPalette>(Rail->GetCategoryCommand(), Rail->GetPaletteCommands()),
			Rail->GetEnabled());
	}

	// Before GenerateWidget, which is what runs UpdateContentForCategory and places this.
	Builder->SetCategoryContent(CreateCategoryContent());

	// World rather than Cell: Cell may be hidden on load, and the builder does not check.
	ToolkitBuilder->SetActivePaletteOnLoad(FNWorldAssemblyEdModePaletteCommands::Get().LoadWorldPalette.Get());
	ToolkitBuilder->UpdateWidget();
}
