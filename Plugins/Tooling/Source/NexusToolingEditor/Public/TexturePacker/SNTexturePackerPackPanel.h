// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "TexturePacker/NTexturePackerTypes.h"
#include "Widgets/SCompoundWidget.h"

class FAssetThumbnailPool;
class SEditableTextBox;
class UTexture2D;
struct FAssetData;

/**
 * The Texture Packer's pack half: four channel slots, and where their combination is written.
 *
 * The output path and name are proposed from what the assigned textures are called and stop being proposed
 * the moment either is typed into, so the common case takes no typing and an uncommon one is not fought with.
 */
class SNTexturePackerPackPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNTexturePackerPackPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Assigns Textures to channels the way an automatic run would, and re-proposes the output. */
	void SetSourceTextures(const TArray<UTexture2D*>& Textures);

private:
	/** One output channel's supply, as the panel holds it between runs. */
	struct FSlotState
	{
		/** Weak, because a texture the panel is holding can be deleted or reloaded while the window sits open. */
		TWeakObjectPtr<UTexture2D> Texture;

		/** Which part of Texture feeds this channel. */
		ENTextureChannelSource Source = ENTextureChannelSource::Auto;

		/** Writes one minus the sampled value. */
		bool bInvert = false;

		/** Ignores the texture's sRGB flag and takes its stored values as they are. */
		bool bTreatAsLinear = false;

		/** Value written where no texture is assigned. */
		float ConstantValue = 0.0f;
	};

	/** Builds the four-column row for one channel. */
	TSharedRef<SWidget> BuildChannelRow(int32 ChannelIndex);

	/** Builds the menu of source-channel choices for one slot. */
	TSharedRef<SWidget> BuildSourceMenu(int32 ChannelIndex);

	/** Re-reads the assigned textures and updates the detected layout, and the proposed name and path. */
	void RefreshDetection();

	//~Slot accessors, all taking the channel they act on
	FString GetSlotObjectPath(int32 ChannelIndex) const;
	void HandleSlotObjectChanged(const FAssetData& AssetData, int32 ChannelIndex);
	FText GetSourceText(int32 ChannelIndex) const;
	ECheckBoxState GetInvertState(int32 ChannelIndex) const;
	void HandleInvertChanged(ECheckBoxState NewState, int32 ChannelIndex);
	ECheckBoxState GetTreatAsLinearState(int32 ChannelIndex) const;
	void HandleTreatAsLinearChanged(ECheckBoxState NewState, int32 ChannelIndex);
	float GetConstantValue(int32 ChannelIndex) const;
	void HandleConstantValueChanged(float NewValue, int32 ChannelIndex);
	EVisibility GetConstantVisibility(int32 ChannelIndex) const;
	bool IsSlotTextureAssigned(int32 ChannelIndex) const;
	//End Slot accessors

	//~Output
	FText GetOutputPathText() const { return FText::FromString(OutputPath); }
	void HandleOutputPathChanged(const FText& NewText);
	FText GetOutputNameText() const { return FText::FromString(OutputName); }
	void HandleOutputNameChanged(const FText& NewText);
	FReply HandleUseSelectedPathClicked();
	FReply HandleResetNamingClicked();
	//End Output

	//~Options
	ECheckBoxState GetSixteenBitState() const;
	void HandleSixteenBitChanged(ECheckBoxState NewState);
	ECheckBoxState GetOverwriteState() const;
	void HandleOverwriteChanged(ECheckBoxState NewState);
	//End Options

	/** @return What the assigned textures were recognized as, for the readout above the output row. */
	FText GetDetectionText() const;

	/** @return The color the detection readout is drawn in - dimmed where nothing was recognized. */
	FSlateColor GetDetectionColor() const;

	/** @return Everything the last run had to say, plus anything standing in the way of the next one. */
	FText GetMessageText() const;

	/** @return Visible only where there is something in GetMessageText to show. */
	EVisibility GetMessageVisibility() const;

	/** @return The color messages are drawn in, red where the last run failed. */
	FSlateColor GetMessageColor() const;

	/** @return true where enough is filled in to run. */
	bool CanPack() const;

	/** Runs the pack, and keeps whatever it had to say. */
	FReply HandlePackClicked();

	/** The four channels in RGBA order. */
	TArray<FSlotState> Slots;

	/** Content-browser folder the result is written to. */
	FString OutputPath;

	/** Name of the generated asset, suffix included. */
	FString OutputName;

	/** Stops the path being re-proposed once it has been typed into. */
	bool bOutputPathEdited = false;

	/** Stops the name being re-proposed once it has been typed into. */
	bool bOutputNameEdited = false;

	/** Write the result at 16 bits per channel. */
	bool bSixteenBit = false;

	/** Replace an asset already standing at the output path. */
	bool bOverwriteExisting = false;

	/** The layout the assigned textures were recognized as, where they were. */
	FNTexturePackedLayout DetectedLayout;

	/** Whether DetectedLayout came from the conventions rather than being composed as a fallback. */
	bool bLayoutRecognized = false;

	/** Warnings and errors from the last run, shown until the next one. */
	TArray<FText> Messages;

	/** Whether the last run failed, which is the difference between a red message and a neutral one. */
	bool bLastRunFailed = false;

	/** Shared with every asset picker in the panel so the thumbnails come off one pool. */
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;

	/** Held so the name box can be pushed back to a proposal when the naming is reset. */
	TSharedPtr<SEditableTextBox> OutputNameBox;

	/** Held so the path box can be pushed back to a proposal when the naming is reset. */
	TSharedPtr<SEditableTextBox> OutputPathBox;
};
