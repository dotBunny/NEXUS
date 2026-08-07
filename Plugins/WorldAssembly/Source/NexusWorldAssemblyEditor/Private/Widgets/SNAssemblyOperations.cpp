// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/SNAssemblyOperations.h"

#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyRegistry.h"
#include "Assembly/NAssemblyOperation.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/** @return The small dim font the secondary lines of a block are set in. */
static FSlateFontInfo GetDetailFont()
{
	return FCoreStyle::GetDefaultFontStyle("Regular", 8);
}

/**
 * @return The text style the report link draws in: the panel's detail font, keeping the link's own coloring.
 * @note SHyperlink takes a whole FTextBlockStyle rather than a font, so matching the lines around it means deriving
 *       one. Built once and shared, since FAppStyle's copy is fixed at editor-style load and cannot change under us.
 */
static const FTextBlockStyle& GetReportLinkTextStyle()
{
	static const FTextBlockStyle Style = FTextBlockStyle(FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText"))
		.SetFont(GetDetailFont());
	return Style;
}

/**
 * @return The last finished run's summary, or nullptr while none has finished since the editor opened.
 * @note Resolved per binding rather than cached, so the idle block reflects a run that finishes while it is already
 *       on screen. The subsystem outlives this widget, but the TOptional it hands back does not survive being
 *       overwritten — never hold the returned pointer past the binding that asked for it.
 */
static const FNAssemblyRunSummary* FindLastRunSummary()
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	if (Subsystem == nullptr) return nullptr;

	const TOptional<FNAssemblyRunSummary>& Summary = Subsystem->GetLastRunSummary();
	return Summary.IsSet() ? &Summary.GetValue() : nullptr;
}

/**
 * @param Height How tall to draw the bar.
 * @param Percent Binding supplying the 0..1 fill.
 * @return A progress bar pinned to Height.
 * @note Height-capped because SProgressBar sizes itself from its style's background brush, which is built for a
 *       dialog rather than a mode panel — left alone, one bar is taller than the two text lines around it.
 */
static TSharedRef<SWidget> CreateBar(const float Height, TAttribute<TOptional<float>> Percent)
{
	return SNew(SBox)
		.HeightOverride(Height)
		[
			SNew(SProgressBar)
			.Percent(MoveTemp(Percent))
		];
}

void SNAssemblyOperations::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(OperationsBox, SVerticalBox)
	];

	Rebuild();

	OperationStateChangedHandle = FNWorldAssemblyRegistry::OnOperationStateChanged.AddSP(
		this, &SNAssemblyOperations::OnOperationStateChanged);

	OperationChannelsChangedHandle = FNWorldAssemblyRegistry::OnOperationChannelsChanged.AddSP(
		this, &SNAssemblyOperations::OnOperationChannelsChanged);
}

SNAssemblyOperations::~SNAssemblyOperations()
{
	FNWorldAssemblyRegistry::OnOperationStateChanged.Remove(OperationStateChangedHandle);
	FNWorldAssemblyRegistry::OnOperationChannelsChanged.Remove(OperationChannelsChangedHandle);
}

void SNAssemblyOperations::OnOperationStateChanged(UNAssemblyOperation* Operation, const ENWorldAssemblyOperationState NewState)
{
	if (Operation == nullptr || !ShouldShowOperation(Operation)) return;

	// Only the two states that change which operations exist need a rebuild. Everything Started/Updated/Finished move
	// is already on an attribute, so those frames redraw without touching the widget tree.
	switch (NewState)
	{
		using enum ENWorldAssemblyOperationState;
	case Registered:
		Rebuild();
		break;
	case Unregistered:
		// Before the rebuild, so the departing operation's bars go with it rather than lingering as orphaned state
		// keyed to a ticket that will never be reissued.
		ChannelStates.Remove(Operation->GetTicket());
		Rebuild();
		break;
	default:
		break;
	}
}

void SNAssemblyOperations::OnOperationChannelsChanged(UNAssemblyOperation* Operation, const TArray<FNStatusChannelUpdate>& Changes)
{
	if (Operation == nullptr || !ShouldShowOperation(Operation)) return;

	FNOperationChannels& Channels = ChannelStates.FindOrAdd(Operation->GetTicket());

	// A rebuild is only owed when the set of channels grows — an update to a channel already on screen is picked up by
	// that bar's attributes on the next frame, and rebuilding for it would rebuild the panel every tick of the graph.
	bool bHasNewChannel = false;
	for (const FNStatusChannelUpdate& Change : Changes)
	{
		if (!Channels.States.Contains(Change.ChannelId))
		{
			Channels.Order.Add(Change.ChannelId);
			bHasNewChannel = true;
		}
		Channels.States.Add(Change.ChannelId,
			{ FText::FromString(Change.Label), FText::FromString(Change.Message), Change.Percent });
	}

	if (bHasNewChannel)
	{
		Rebuild();
	}
}

void SNAssemblyOperations::Rebuild()
{
	if (!OperationsBox.IsValid()) return;

	OperationsBox->ClearChildren();

	int32 ShownCount = 0;
	for (UNAssemblyOperation* Operation : FNWorldAssemblyRegistry::GetOperations())
	{
		if (Operation == nullptr || !ShouldShowOperation(Operation)) continue;

		OperationsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, ShownCount == 0 ? 0.0f : 6.0f, 0.0f, 0.0f)
			[
				CreateOperationBlock(Operation)
			];

		ShownCount++;
	}

	if (ShownCount > 0) return;

	OperationsBox->AddSlot()
		.AutoHeight()
		[
			CreateIdleBlock()
		];
}

TSharedRef<SWidget> SNAssemblyOperations::CreateIdleBlock()
{
	// Visibility for anything that only makes sense once a run has finished.
	auto SummaryVisibility = []()
	{
		return FindLastRunSummary() != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
	};

	return SNew(SVerticalBox)

		// The idle state says so rather than collapsing: a section that empties itself leaves the panel jumping by its
		// height every time an operation finishes, and nothing to tell you the section is watching.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Operations_None", "No operations running."))
			.Font(GetDetailFont())
			.ColorAndOpacity(FStyleColors::White25)
		]

		// Result title, with the same icon vocabulary the completion toast uses so the two read as one report. The
		// cells/duration counts the result also carries are deliberately not shown: the message below already states
		// them in prose, and printing both puts the same two numbers on screen twice.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			.Visibility_Lambda(SummaryVisibility)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(12.0f)
				.HeightOverride(12.0f)
				[
					SNew(SImage)
					.Image_Lambda([]() -> const FSlateBrush*
					{
						const FNAssemblyRunSummary* Summary = FindLastRunSummary();
						if (Summary == nullptr) return nullptr;

						// The plain X the cancel affordance above uses, rather than the error icon: a run the user
						// stopped is not a failed one, and reusing the glyph they clicked names the cause.
						if (Summary->bCancelled) return FAppStyle::GetBrush("Icons.X");

						if (Summary->bWarning) return FAppStyle::GetBrush("Icons.WarningWithColor");
						return Summary->bSuccess
							? FAppStyle::GetBrush("Icons.SuccessWithColor")
							: FAppStyle::GetBrush("Icons.ErrorWithColor");
					})
					// Left untinted: the WithColor brushes above carry their own, and tinting would flatten all three
					// to one color. Icons.X is already drawn in the foreground color.
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([]()
				{
					const FNAssemblyRunSummary* Summary = FindLastRunSummary();
					return Summary != nullptr ? Summary->Title : FText::GetEmpty();
				})
				.Font(GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 1.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				const FNAssemblyRunSummary* Summary = FindLastRunSummary();
				return Summary != nullptr ? Summary->Message : FText::GetEmpty();
			})
			.Visibility_Lambda([]()
			{
				const FNAssemblyRunSummary* Summary = FindLastRunSummary();
				return Summary != nullptr && !Summary->Message.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.Font(GetDetailFont())
			.ColorAndOpacity(FStyleColors::White25)
			// Wrapped rather than ellipsized: this is the one line here carrying detail the numbers above do not,
			// and the panel is narrow enough that a single line of it would be cut to nothing useful.
			.AutoWrapText(true)
		]

		// Opens the report the run wrote, the same file the completion toast links.
		+ SVerticalBox::Slot()
		.AutoHeight()
		// Left rather than the slot default of Fill: SHyperlink draws its underline across its whole allotted width,
		// so a filled slot underlines the empty space past the filename as well as the filename.
		.HAlign(HAlign_Left)
		.Padding(16.0f, 2.0f, 0.0f, 0.0f)
		[
			SNew(SHyperlink)
			.Style(FAppStyle::Get(), "HoverOnlyHyperlink")
			.TextStyle(&GetReportLinkTextStyle())
			.Text_Lambda([]()
			{
				const FNAssemblyRunSummary* Summary = FindLastRunSummary();
				return Summary != nullptr
					? FText::FromString(FPaths::GetCleanFilename(Summary->ReportFilePath))
					: FText::GetEmpty();
			})
			.ToolTipText(LOCTEXT("Operations_LastRunReport_Tooltip", "Open the report this run wrote."))
			.Visibility_Lambda([]()
			{
				const FNAssemblyRunSummary* Summary = FindLastRunSummary();
				return Summary != nullptr && !Summary->ReportFilePath.IsEmpty()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			.OnNavigate_Lambda([]()
			{
				const FNAssemblyRunSummary* Summary = FindLastRunSummary();
				if (Summary == nullptr || Summary->ReportFilePath.IsEmpty()) return;

				FPlatformProcess::LaunchFileInDefaultExternalApplication(
					*FPaths::ConvertRelativePathToFull(Summary->ReportFilePath));
			})
		];
}

TSharedRef<SWidget> SNAssemblyOperations::CreateOperationBlock(UNAssemblyOperation* Operation) const
{
	// Weak, because the row outlives any single operation: the registry holds operations only while they are alive, and
	// a block is not torn down until the rebuild that follows the unregister broadcast.
	const TWeakObjectPtr<UNAssemblyOperation> WeakOperation = Operation;
	const int32 Ticket = Operation->GetTicket();

	const TSharedRef<SVerticalBox> Block = SNew(SVerticalBox)

		// Name on the left, task counts and the cancel affordance on the right.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([WeakOperation]()
				{
					return WeakOperation.IsValid() ? WeakOperation->GetDisplayName() : FText::GetEmpty();
				})
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([WeakOperation]()
				{
					if (!WeakOperation.IsValid()) return FText::GetEmpty();

					// Blank rather than "0/0" while the graph is still being built: a total of zero means the task
					// count is not known yet, not that there is no work.
					const FIntVector2 Tasks = WeakOperation->GetCachedTaskStatusCounts();
					return Tasks.Y == 0
						? FText::GetEmpty()
						: FText::FromString(FString::Printf(TEXT("%d/%d"), Tasks.X, Tasks.Y));
				})
				.Font(GetDetailFont())
				.ColorAndOpacity(FStyleColors::White25)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
				.ContentPadding(0.0f)
				.ToolTipText(LOCTEXT("Operations_Cancel", "Cancel this operation."))
				.Visibility_Lambda([WeakOperation]()
				{
					return WeakOperation.IsValid() && WeakOperation->IsRunning() ? EVisibility::Visible : EVisibility::Hidden;
				})
				.OnClicked_Lambda([WeakOperation]()
				{
					if (WeakOperation.IsValid())
					{
						WeakOperation->Cancel();
					}
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.X"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]

		// Combined task + channel progress, so the bar keeps moving through a long task rather than freezing between
		// task-completion milestones.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 0.0f)
		[
			CreateBar(8.0f, TAttribute<TOptional<float>>::CreateLambda([WeakOperation]() -> TOptional<float>
			{
				return WeakOperation.IsValid() ? WeakOperation->GetCombinedProgress() : 0.0f;
			}))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([WeakOperation]()
			{
				return WeakOperation.IsValid() ? FText::FromString(WeakOperation->GetDisplayMessage()) : FText::GetEmpty();
			})
			.Visibility_Lambda([WeakOperation]()
			{
				return WeakOperation.IsValid() && !WeakOperation->GetDisplayMessage().IsEmpty()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			.Font(GetDetailFont())
			.ColorAndOpacity(FStyleColors::White25)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];

	if (const FNOperationChannels* Channels = ChannelStates.Find(Ticket))
	{
		for (const int32 ChannelId : Channels->Order)
		{
			Block->AddSlot()
				.AutoHeight()
				.Padding(8.0f, 2.0f, 0.0f, 0.0f)
				[
					CreateChannelBar(Ticket, ChannelId)
				];
		}
	}

	return Block;
}

TSharedRef<SWidget> SNAssemblyOperations::CreateChannelBar(const int32 Ticket, const int32 ChannelId) const
{
	// Every binding below resolves the channel through this, rather than capturing the update by value: the map is
	// what the delta broadcast writes into, so reading it each frame is what makes an open channel animate.
	auto FindChannel = [this, Ticket, ChannelId]() -> const FNChannelSnapshot*
	{
		const FNOperationChannels* Channels = ChannelStates.Find(Ticket);
		return Channels != nullptr ? Channels->States.Find(ChannelId) : nullptr;
	};

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text_Lambda([FindChannel]()
				{
					const FNChannelSnapshot* Channel = FindChannel();
					return Channel != nullptr ? Channel->Label : FText::GetEmpty();
				})
				.Font(GetDetailFont())
				.ColorAndOpacity(FStyleColors::White25)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([FindChannel]()
				{
					const FNChannelSnapshot* Channel = FindChannel();
					return Channel != nullptr ? Channel->Message : FText::GetEmpty();
				})
				.Font(GetDetailFont())
				.ColorAndOpacity(FStyleColors::White25)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 1.0f, 0.0f, 0.0f)
		[
			CreateBar(4.0f, TAttribute<TOptional<float>>::CreateLambda([FindChannel]() -> TOptional<float>
			{
				const FNChannelSnapshot* Channel = FindChannel();
				return Channel != nullptr ? Channel->Percent : 0.0f;
			}))
		];
}

bool SNAssemblyOperations::ShouldShowOperation(const UNAssemblyOperation* Operation)
{
	return Operation->GetFName() != NEXUS::WorldAssembly::Operations::EditorMode;
}

#undef LOCTEXT_NAMESPACE
