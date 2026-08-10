// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/** Log category used by all NexusCoreEditor logging. */
DECLARE_LOG_CATEGORY_EXTERN(LogNexusCoreEditor, Log, All);

/**
 * Identifiers used when trying to map to ToolMenus.
 */
namespace NEXUS::CoreEditor::ToolMenus
{
	inline FName LevelEditorToolBarUser = TEXT("LevelEditor.LevelEditorToolBar.User");
}

/**
 * Tuning shared by everything that decides whether a terrain build has finished.
 */
namespace NEXUS::CoreEditor::Terrain
{
	/**
	 * How long a level's terrain fingerprint must hold still before the build behind it is treated as finished.
	 *
	 * Public because settling gets inferred in two ways: passively, by a caller polling FNTerrainUtils::ComputeFingerprint
	 * on its own tick, and actively by FNTerrainUtils::WaitForSettle, which pumps. The two disagreeing would mean a
	 * button re-enabling at a different moment than a wait returns, so both read this.
	 * @note Too short and a gap between two sections landing reads as the end of the build; too long and every
	 *       calculation pays the difference. Sections land in quick succession once the build is moving.
	 */
	inline constexpr double SettleSeconds = 0.35;

	/** How long FNTerrainUtils::WaitForSettle blocks before giving up and reporting failure. */
	inline constexpr double DefaultSettleTimeoutSeconds = 30.0;
}