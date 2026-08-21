// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NToolingEditorStyle.h"
#include "Framework/Commands/Commands.h"

/**
 * Keyboard-bindable commands published by NexusToolingEditor.
 *
 * One shared binding context rather than one per feature, so every keybound Tooling command groups under
 * a single heading in Editor Preferences > Keyboard Shortcuts instead of each adding its own category.
 *
 * Distinct from FNToolingEditorCommands, which despite the name is not a TCommands at all — it is the
 * ToolMenus menu builder and carries no chords.
 * @see <a href="https://nexus-framework.com/docs/plugins/tooling/editor-types/tooling-editor-bindings/">FNToolingEditorBindings</a>
 */
class NEXUSTOOLINGEDITOR_API FNToolingEditorBindings final : public TCommands<FNToolingEditorBindings>
{
public:
	FNToolingEditorBindings()
		: TCommands<FNToolingEditorBindings>(
			TEXT("NToolingEditorBindings"),
			NSLOCTEXT("Contexts", "NToolingEditorBindings", "NEXUS: Tooling"),
			NAME_None,
			FNToolingEditorStyle::GetStyleSetName())
	{
	}

	//~TCommands
	virtual void RegisterCommands() override;
	//End TCommands

	/**
	 * Bind the registered commands onto the level editor's global command list, so their chords fire
	 * regardless of which panel holds focus within the level editor window.
	 */
	static void MapActions();

	/** Drop those bindings again, leaving no action pointing at this module once it unloads. */
	static void UnmapActions();

	/** Captures the active level viewport at the multiplier held on UNToolingEditorUserSettings. */
	TSharedPtr<FUICommandInfo> CommandInfo_QuickHighResScreenshot;
};
