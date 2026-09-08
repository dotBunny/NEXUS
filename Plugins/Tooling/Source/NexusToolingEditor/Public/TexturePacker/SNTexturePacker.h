// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SNTexturePackerPackPanel;
class SNTexturePackerUnpackPanel;
class SWidgetSwitcher;
class UTexture2D;

/** Which half of the Texture Packer window is showing. */
enum class ENTexturePackerMode : uint8
{
	/** Several maps in, one packed texture out. */
	Pack = 0,
	/** One packed texture in, one asset per map it holds out. */
	Unpack = 1
};

/**
 * The Texture Packer window: a mode switch, and whichever of the two panels it has selected.
 *
 * The shell owns nothing but the switch. Each panel keeps its own state, so moving between the two modes and
 * back finds the work where it was left - which matters, because packing a set and then checking an existing
 * one is a normal way round to work.
 */
class SNTexturePacker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNTexturePacker) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Shows the unpack panel with Texture already loaded into it. */
	void OpenForUnpack(UTexture2D* Texture);

	/** Shows the pack panel with Textures already assigned to channels. */
	void OpenForPack(const TArray<UTexture2D*>& Textures);

private:
	/** @return The mode the switch is on, for the segmented control to read back. */
	ENTexturePackerMode GetMode() const { return Mode; }

	/** @return The mode as the switcher's slot index; the enum's values are that index. */
	int32 GetModeIndex() const;

	/** Moves the switcher, and the switch with it. */
	void SetMode(ENTexturePackerMode InMode);

	/** Which panel is showing. */
	ENTexturePackerMode Mode = ENTexturePackerMode::Pack;

	/** Holds both panels, showing one. */
	TSharedPtr<SWidgetSwitcher> Switcher;

	/** The pack half, built once and kept. */
	TSharedPtr<SNTexturePackerPackPanel> PackPanel;

	/** The unpack half, built once and kept. */
	TSharedPtr<SNTexturePackerUnpackPanel> UnpackPanel;
};
