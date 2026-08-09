// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdModeToolkit.h"

#include "IAssetViewport.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "Commands/NWorldAssemblyEditorCellCommands.h"
#include "Commands/NWorldAssemblyEditorJunctionCommands.h"
#include "Commands/NWorldAssemblyEditorOrganCommands.h"
#include "Commands/NWorldAssemblyEditorWorldCommands.h"
#include "InteractiveTool.h"
#include "Commands/NWorldAssemblyEditorCellDataCommands.h"
#include "Overlay/SDraggableBoxOverlay.h"
#include "Rails/NWorldAssemblyEdModeCellDataRail.h"
#include "Rails/NWorldAssemblyEdModeCellRail.h"
#include "Rails/NWorldAssemblyEdModeJunctionRail.h"
#include "Rails/NWorldAssemblyEdModeOrganRail.h"
#include "Rails/NWorldAssemblyEdModeWorldRail.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SNWorldAssemblyRail.h"
#include "Widgets/SNWorldAssemblyRailPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/** Inset of the category strip from the viewport's top-left corner. Matches what Mesh Terrain pins its palette at. */
static constexpr float RailInset = 16.0f;

/** Where the panel opens the first time, to the right of the strip and level with its top. */
static constexpr float PanelInitialOffsetX = 116.0f;

/** Smallest the user may drag the panel, below which the titled groups start ellipsizing their labels. */
static constexpr float PanelMinimumWidth = 260.0f;

/** Largest the user may drag the panel; past this it stops reading as an overlay. */
static constexpr float PanelMaximumWidth = 520.0f;

/** Width the panel opens at before the user has resized it. */
static constexpr float PanelDefaultWidth = 300.0f;

void FNWorldAssemblyEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	RegisterRails();
	CreateOverlays();

	// After Init, which is what establishes the host this subscribes to.
	if (IsHosted())
	{
		GetToolkitHost()->OnActiveViewportChanged().AddSP(this, &FNWorldAssemblyEdModeToolkit::OnActiveViewportChanged);
	}
}

FNWorldAssemblyEdModeToolkit::~FNWorldAssemblyEdModeToolkit()
{
	// IsHosted before GetToolkitHost, not a check on what it returns: GetToolkitHost hands back a TSharedRef and
	// pins the weak host inside itself, so it asserts rather than returning null once the host is gone. On editor
	// shutdown it always is — the world is cleaned up first, which exits every mode after the level editor has
	// already dropped the toolkit host.
	if (IsHosted())
	{
		if (RailOverlay.IsValid())
		{
			GetToolkitHost()->RemoveViewportOverlayWidget(RailOverlay.ToSharedRef(), OverlayViewport);
		}

		if (PanelOverlay.IsValid())
		{
			GetToolkitHost()->RemoveViewportOverlayWidget(PanelOverlay.ToSharedRef(), OverlayViewport);
		}
	}

	RailOverlay.Reset();
	PanelOverlay.Reset();
	OverlayViewport.Reset();
}

FText FNWorldAssemblyEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("NWorldAssemblyEdModeToolkit_Name", "World Assembly");
}

void FNWorldAssemblyEdModeToolkit::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolStarted(Manager, Tool);

	ActiveToolName = Tool != nullptr ? Tool->GetToolInfo().ToolDisplayName : FText::GetEmpty();
}

void FNWorldAssemblyEdModeToolkit::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolEnded(Manager, Tool);

	ActiveToolName = FText::GetEmpty();
}

void FNWorldAssemblyEdModeToolkit::RegisterRails()
{
	// The rail buttons resolve their actions against the toolkit's own command list, so fold in the per-area lists the
	// commands were already mapped into. This is also what scopes them to the mode: the Organ bindings used to be
	// appended to the level editor's global action list, so their chords fired whether or not the mode was open.
	const TSharedRef<FUICommandList> ToolkitCommandList = GetToolkitCommands();
	ToolkitCommandList->Append(FNWorldAssemblyEditorWorldCommands::Get().CommandList.ToSharedRef());
	ToolkitCommandList->Append(FNWorldAssemblyEditorCellCommands::Get().CommandList.ToSharedRef());
	ToolkitCommandList->Append(FNWorldAssemblyEditorCellDataCommands::Get().CommandList.ToSharedRef());
	ToolkitCommandList->Append(FNWorldAssemblyEditorJunctionCommands::Get().CommandList.ToSharedRef());
	ToolkitCommandList->Append(FNWorldAssemblyEditorOrganCommands::Get().CommandList.ToSharedRef());

	// Order here is the order of the buttons on the strip.
	TArray<TSharedRef<FNWorldAssemblyEdModeRail>> Rails;
	Rails.Add(MakeShared<FNWorldAssemblyEdModeWorldRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeCellRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeCellDataRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeJunctionRail>(ToolkitCommandList));
	Rails.Add(MakeShared<FNWorldAssemblyEdModeOrganRail>(ToolkitCommandList));

	RailState = MakeShared<FNWorldAssemblyRailState>(MoveTemp(Rails));
}

void FNWorldAssemblyEdModeToolkit::CreateOverlays()
{
	// See the destructor: GetToolkitHost asserts rather than returning null, so the guard has to be IsHosted.
	if (!IsHosted()) return;

	// SelfHitTestInvisible on both, so the boxes themselves do not eat viewport clicks — only their content does.
	// Without it each box's full extent, most of which is empty, would swallow everything behind it.

	// Pinned rather than draggable. This is the mode's anchor: it is what the user aims at to change category, and a
	// fixed target is worth more than a movable one. Mesh Terrain pins its submode palette the same way.
	SAssignNew(RailOverlay, UE::ToolWidgets::SDraggableBoxOverlay)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.InitialAlignmentOffset(FVector2f(RailInset, RailInset))
		.Cursor(EMouseCursor::Default)
		.Draggable(false)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.Content()
		[
			SNew(SBorder)
			.BorderImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.RailBackground"))
			// The largest of the three terms deciding the strip's gutter, the other two being
			// WorldAssemblyEd.CategoryToolBar's ButtonPadding of 2 and its IconPadding of 4 — together putting every
			// icon 13 off the rounded edge, which is what Mesh Terrain's palette measures out to.
			//
			// Needed at all rather than folding the whole gutter into the toolbar: it draws no background or padding
			// of its own, and at zero here the corners read as square, because Slate does not clip a child to the
			// rounding of the brush behind it.
			.Padding(7.0f)
			[
				SNew(SNWorldAssemblyRail)
				.State(RailState)
			]
		];

	const UNWorldAssemblyEditorUserSettings* Settings = GetDefault<UNWorldAssemblyEditorUserSettings>();
	const FToolWidget_DragBoxPosition SavedPosition = Settings->EdModePanelPosition;
	const float SavedWidth = Settings->EdModePanelWidth;

	SAssignNew(PanelOverlay, UE::ToolWidgets::SDraggableBoxOverlay)
		.HAlign(SavedPosition.HAlign)
		.VAlign(SavedPosition.VAlign)
		.InitialAlignmentOffset(SavedPosition.RelativeOffset)
		.Cursor(EMouseCursor::Default)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.Resizable(UE::ToolWidgets::EResizeEdges::Right)
		.WidthOverride(SavedWidth > 0.0f ? SavedWidth : PanelDefaultWidth)
		.MinimumBoxWidth(PanelMinimumWidth)
		.MaximumBoxWidth(PanelMaximumWidth)
		.OnUserDraggedToNewPosition_Lambda([this]() { SavePanelLayout(); })
		.OnUserResized_Lambda([this]() { SavePanelLayout(); })
		.Content()
		[
			SNew(SBorder)
			// The darker half of the pair; see WorldAssemblyEd.RailBackground for the lighter one on the strip.
			.BorderImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.PanelBackground"))
			.Padding(4.0f)
			[
				SNew(SNWorldAssemblyRailPanel)
				.State(RailState)
				.ActiveToolName_Lambda([this]() { return ActiveToolName; })
			]
		];

	GetToolkitHost()->AddViewportOverlayWidget(RailOverlay.ToSharedRef(), OverlayViewport);
	GetToolkitHost()->AddViewportOverlayWidget(PanelOverlay.ToSharedRef(), OverlayViewport);
}

void FNWorldAssemblyEdModeToolkit::SavePanelLayout() const
{
	if (!PanelOverlay.IsValid()) return;

	UNWorldAssemblyEditorUserSettings* Settings = GetMutableDefault<UNWorldAssemblyEditorUserSettings>();
	Settings->EdModePanelPosition = PanelOverlay->GetDragBoxPosition();
	Settings->EdModePanelWidth = PanelOverlay->GetWidthOverride().Get(0.0f);
	Settings->SaveConfig();
}

void FNWorldAssemblyEdModeToolkit::OnActiveViewportChanged(TSharedPtr<IAssetViewport> OldViewport, TSharedPtr<IAssetViewport> NewViewport)
{
	if (!IsHosted()) return;

	// Only when it was deselected rather than destroyed — removing from a viewport that is already gone is what the
	// null check here is avoiding, not a missing overlay.
	if (OldViewport.IsValid())
	{
		if (RailOverlay.IsValid()) GetToolkitHost()->RemoveViewportOverlayWidget(RailOverlay.ToSharedRef(), OldViewport);
		if (PanelOverlay.IsValid()) GetToolkitHost()->RemoveViewportOverlayWidget(PanelOverlay.ToSharedRef(), OldViewport);
	}

	OverlayViewport = NewViewport;

	if (RailOverlay.IsValid()) GetToolkitHost()->AddViewportOverlayWidget(RailOverlay.ToSharedRef(), OverlayViewport);
	if (PanelOverlay.IsValid()) GetToolkitHost()->AddViewportOverlayWidget(PanelOverlay.ToSharedRef(), OverlayViewport);
}

#undef LOCTEXT_NAMESPACE
