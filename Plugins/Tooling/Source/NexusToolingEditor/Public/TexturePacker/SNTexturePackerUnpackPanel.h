// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "TexturePacker/NTexturePackerJob.h"
#include "Widgets/SCompoundWidget.h"

class FAssetThumbnailPool;
class SVerticalBox;
class UTexture2D;
struct FAssetData;

/**
 * The Texture Packer's unpack half: one packed texture, and a row per map it turns out to hold.
 *
 * What it holds is read from its name and can be overridden, because a texture that was named badly is
 * exactly the one somebody needs to take apart. Channels carrying the same map are shown as one row - a color
 * map packed across RGB is one texture to recover, not three.
 */
class SNTexturePackerUnpackPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNTexturePackerUnpackPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Loads Texture into the panel and re-reads what it holds. */
	void SetSourceTexture(UTexture2D* Texture);

private:
	/** One row of the outputs list: what will be written, and whether its name is still being proposed. */
	struct FOutputState
	{
		/** The output as the job will receive it. */
		FNTextureUnpackOutput Output;

		/** Stops the name being re-proposed once it has been typed into. */
		bool bNameEdited = false;
	};

	/** Re-reads the source texture's name, and rebuilds the outputs from what it says. */
	void RefreshDetection();

	/** Rebuilds the output rows to match Outputs. */
	void RebuildOutputRows();

	/** Builds one output row. */
	TSharedRef<SWidget> BuildOutputRow(int32 OutputIndex);

	/** Builds the menu of layouts the source can be read as. */
	TSharedRef<SWidget> BuildLayoutMenu();

	/** Builds the menu of roles one output can be assigned. */
	TSharedRef<SWidget> BuildRoleMenu(int32 OutputIndex);

	/** Replaces Outputs with the runs of Layout, naming each for the role it holds. */
	void ApplyLayout(const FNTexturePackedLayout& InLayout, bool bRecognized);

	/** Switches off any output whose channels all hold one value, where the settings ask for that. */
	void DisableUniformOutputs();

	//~Source
	FString GetSourceObjectPath() const;
	void HandleSourceObjectChanged(const FAssetData& AssetData);
	//End Source

	//~Output rows, all taking the row they act on
	ECheckBoxState GetOutputEnabledState(int32 OutputIndex) const;
	void HandleOutputEnabledChanged(ECheckBoxState NewState, int32 OutputIndex);
	FText GetOutputRoleText(int32 OutputIndex) const;
	FText GetOutputChannelText(int32 OutputIndex) const;
	FText GetOutputNameText(int32 OutputIndex) const;
	void HandleOutputNameChanged(const FText& NewText, int32 OutputIndex);
	//End Output rows

	//~Output location and options
	FText GetOutputPathText() const { return FText::FromString(OutputPath); }
	void HandleOutputPathChanged(const FText& NewText);
	FReply HandleUseSelectedPathClicked();
	ECheckBoxState GetSixteenBitState() const;
	void HandleSixteenBitChanged(ECheckBoxState NewState);
	ECheckBoxState GetOverwriteState() const;
	void HandleOverwriteChanged(ECheckBoxState NewState);
	ECheckBoxState GetTreatAsLinearState() const;
	void HandleTreatAsLinearChanged(ECheckBoxState NewState);
	//End Output location and options

	/** @return What the source was recognized as, for the readout above the rows. */
	FText GetDetectionText() const;

	/** @return The color the detection readout is drawn in - dimmed where nothing was recognized. */
	FSlateColor GetDetectionColor() const;

	/** @return Everything the last run had to say. */
	FText GetMessageText() const;

	/** @return Visible only where there is something in GetMessageText to show. */
	EVisibility GetMessageVisibility() const;

	/** @return The color messages are drawn in, red where the last run failed. */
	FSlateColor GetMessageColor() const;

	/** @return true where enough is filled in to run. */
	bool CanUnpack() const;

	/** Runs the unpack, and keeps whatever it had to say. */
	FReply HandleUnpackClicked();

	/** The packed texture being taken apart. */
	TWeakObjectPtr<UTexture2D> SourceTexture;

	/** The source's name with its suffix removed, which every output is named from. */
	FString BaseName;

	/** How the source is being read. */
	FNTexturePackedLayout Layout;

	/** Whether Layout came from the source's name rather than being chosen by hand. */
	bool bLayoutRecognized = false;

	/** One row per map the source holds. */
	TArray<FOutputState> Outputs;

	/** Content-browser folder the results are written to. */
	FString OutputPath;

	/** Stops the path being re-proposed once it has been typed into. */
	bool bOutputPathEdited = false;

	/** Write the results at 16 bits per channel. */
	bool bSixteenBit = false;

	/** Replace assets already standing at the output paths. */
	bool bOverwriteExisting = false;

	/** Take the source's stored values as they are, ignoring its sRGB flag. */
	bool bTreatAsLinear = false;

	/** Warnings and errors from the last run, shown until the next one. */
	TArray<FText> Messages;

	/** Whether the last run failed. */
	bool bLastRunFailed = false;

	/** Holds the output rows, cleared and repopulated whenever the layout changes. */
	TSharedPtr<SVerticalBox> OutputRows;

	/** Backs the source picker's thumbnail. */
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;
};
