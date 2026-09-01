// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EdMode/NWorldAssemblyRails.h"
#include "Toolkits/BaseToolkit.h"
#include "Widgets/SNullWidget.h"

class IAssetViewport;

namespace UE::ToolWidgets
{
	class SDraggableBoxOverlay;
}

/**
 * Toolkit for UNWorldAssemblyEdMode, hosting the mode's UI as a viewport overlay rather than a docked panel.
 *
 * The mode lives in two boxes floating over the level viewport, both fed by one shared FNWorldAssemblyRailState:
 * SNWorldAssemblyRail, a pinned strip of category buttons — World, Cell, Junction, Organ — and
 * SNWorldAssemblyRailPanel, a draggable, resizable window showing whichever category the strip has selected. The Mode
 * Toolbox tab is never requested, so nothing of this mode lives in the level editor's side panels.
 *
 * @note The rail order is set by the Rails.Add sequence in RegisterRails, which is the one place to change it.
 * @note Splitting the strip from its content is what Epic's Mesh Terrain mode does in 5.8, and for the reason its
 *       palette is pinned while its details window is not: the strip is a target the user aims at, the window is
 *       something they move out of the way. This class only positions the two, persists where the user put the
 *       movable one, and keeps both attached to whichever viewport is active.
 * @see <a href="https://nexus-framework.com/docs/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyModeToolkit final : public FModeToolkit
{
public:
	/**
	 * @note Pulls the rail back off the viewport. An overlay outlives the toolkit that added it — the host holds its
	 *       own reference — so one left behind survives mode exit and stacks a second copy on re-entry.
	 * @remark Guarded on IsHosted rather than on what GetToolkitHost returns; see the definition for why the obvious
	 *         null check does not work.
	 */
	virtual ~FNWorldAssemblyModeToolkit() override;

	//~FModeToolkit
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;

	/**
	 * @return A name unique to this toolkit.
	 * @note Must not be left at FModeToolkit's "EditorModeToolkit" default. FLevelEditorModeUILayer opens a
	 *       FToolMenuOwnerScoped keyed on this name when it starts hosting a toolkit, and unregisters that owner when
	 *       hosting finishes — so two toolkits sharing a name tear down each other's menu extensions.
	 */
	virtual FName GetToolkitFName() const override { return FName("NWorldAssemblyEdModeToolkit"); }

	virtual FText GetBaseToolkitName() const override;

	/**
	 * @return Nothing; the mode has no docked panel to fill.
	 * @note Only ever asked for by FModeToolkit::CreatePrimaryModePanel, which is the Mode Toolbox tab's spawner —
	 *       and RequestModeUITabs never registers it, so this is unreachable in practice. Returning null rather than
	 *       relying on that keeps the mode from painting an empty panel if it ever becomes reachable again.
	 */
	virtual TSharedPtr<SWidget> GetInlineContent() const override { return SNullWidget::NullWidget; }

	/**
	 * Suppress the Mode Toolbox tab entirely.
	 * @note This is what makes the mode viewport-only, and it is a clean suppression rather than a hack: the base
	 *       binds FMinorTabConfig::OnSpawnTab here, and FAssetEditorModeUILayer::CanSpawnStoredTab gates spawning on
	 *       that delegate being bound. Leaving it unbound makes InvokeUI's TryInvokeTab refuse, and the tab's
	 *       SetAutoGenerateMenuEntry(false) keeps it out of the window menu too.
	 * @remark Deliberately does not call Super. Mesh Terrain mode suppresses the same way.
	 */
	virtual void RequestModeUITabs() override {}
	//End FModeToolkit

private:
	/** Construct the rails, in the order their buttons appear, fold the per-area command lists into the toolkit's, and
	 *  seed the state the two overlays share. */
	void RegisterRails();

	/** Build both overlays, restore where the user last put the panel, and attach them to the active viewport. */
	void CreateOverlays();

	/** Write the panel's position and width back to user settings. Bound to its drag and resize events. */
	void SavePanelLayout() const;

	/**
	 * Move the rail to the viewport that just became active.
	 *
	 * @param OldViewport Viewport losing focus, or null when it was destroyed rather than deselected.
	 * @param NewViewport Viewport gaining focus.
	 * @note An overlay belongs to one viewport, so without this both boxes stay behind on a viewport the user has
	 *       left — and since they are the mode's only UI, the mode would appear to vanish on any viewport layout
	 *       change.
	 */
	void OnActiveViewportChanged(TSharedPtr<IAssetViewport> OldViewport, TSharedPtr<IAssetViewport> NewViewport);

	/** Which category is selected and which are worth offering; read by both overlays. */
	TSharedPtr<FNWorldAssemblyRails> RailState;

	/** The pinned category strip. */
	TSharedPtr<UE::ToolWidgets::SDraggableBoxOverlay> RailOverlay;

	/** The draggable window showing the selected category's content. */
	TSharedPtr<UE::ToolWidgets::SDraggableBoxOverlay> PanelOverlay;

	/** Viewport both overlays are currently attached to, or null for the toolkit host's default. */
	TSharedPtr<IAssetViewport> OverlayViewport;

};
