// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyEdModeToolkit.h"

#include "IAssetViewport.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "Overlay/SDraggableBoxOverlay.h"
#include "EdMode/NWorldAssemblyEdModeCellDataRail.h"
#include "EdMode/NWorldAssemblyEdModeCellRail.h"
#include "EdMode/NWorldAssemblyEdModeJunctionRail.h"
#include "EdMode/NWorldAssemblyEdModeOrganRail.h"
#include "EdMode/NWorldAssemblyEdModeWorldRail.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SNWorldAssemblyRail.h"
#include "Widgets/SNWorldAssemblyRailPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/** Inset of the category strip from the viewport's top-left corner. Matches what Mesh Terrain pins its palette at. */
static constexpr float RailInset = 16.0f;

/** Smallest the user may drag the panel, below which the titled groups start ellipsizing their labels. */
static constexpr float PanelMinimumWidth = 165.0f;

/** Largest the user may drag the panel; past this it stops reading as an overlay. */
static constexpr float PanelMaximumWidth = 232.0f;

/** Width the panel opens at before the user has resized it. */
static constexpr float PanelDefaultWidth = 232.0f;

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

void FNWorldAssemblyEdModeToolkit::RegisterRails()
{
	// The rail buttons resolve their actions against the toolkit's own command list, so fold in the per-category lists
	// the commands were already mapped into. This is also what scopes them to the mode: the Organ bindings used to be
	// appended to the level editor's global action list, so their chords fired whether or not the mode was open.
	//
	// Every list is folded in before any rail is built, and a rail's own buttons resolve against the union rather than
	// against its category alone — which is what lets a category offer a command another one declared.
	const TSharedRef<FUICommandList> ToolkitCommandList = GetToolkitCommands();
	ToolkitCommandList->Append(FNWorldAssemblyEdModeWorldRail::GetCommandList());
	ToolkitCommandList->Append(FNWorldAssemblyEdModeCellRail::GetCommandList());
	ToolkitCommandList->Append(FNWorldAssemblyEdModeCellDataRail::GetCommandList());
	ToolkitCommandList->Append(FNWorldAssemblyEdModeJunctionRail::GetCommandList());
	ToolkitCommandList->Append(FNWorldAssemblyEdModeOrganRail::GetCommandList());

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
		// The whole box goes, not just its content: clicking the lit rail button closes the panel, and a collapsed
		// switcher inside a live box would leave its background and padding drawn over the viewport with nothing in
		// them. Collapsed rather than Hidden so it stops taking space, and the box keeps its position for reopening.
		//
		// The state is captured rather than reached through this: an overlay can outlive the toolkit that added it, and
		// a bound attribute is read on paint. Both widgets inside hold their own copy the same way.
		.Visibility_Lambda([State = RailState]()
		{
			const bool bHasCategory = State.IsValid() && State->GetActiveIndex() != INDEX_NONE;
			return bHasCategory ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
		})
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
			// The panel's gutter is 8 on every side, and these two numbers are whatever is left of that once the groups
			// inside have carried their share: they claim 4 across (the inset their buttons and backings line up on)
			// and GroupVerticalInset down, so this makes up 4 and 2 respectively.
			//
			// Which means this moves whenever GroupVerticalInset does — that constant sets the gap between groups, and
			// what it does not spend on the panel's own top and bottom lands here. See NWorldAssemblyEdModeRail.cpp.
			.Padding(FMargin(4.0f, 2.0f))
			[
				SNew(SNWorldAssemblyRailPanel)
				.State(RailState)
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
