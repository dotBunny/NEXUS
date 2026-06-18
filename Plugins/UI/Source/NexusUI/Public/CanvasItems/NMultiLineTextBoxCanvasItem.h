// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "NUIMinimal.h"
#include "Engine/Font.h"
#include "Types/NSeverity.h"

/**
 * A buffered, multi-line text box rendered to a canvas by FNCanvasUtils.
 *
 * Accumulates lines with per-line color and font size, tracks a severity that drives the border color, and
 * lazily measures/lays out its content (via the private Process pass) when marked dirty.
 */
struct FNMultiLineTextBoxCanvasItem
{
	friend class FNCanvasUtils;

	/** Removes all lines and resets colors, severity, and cached layout state. */
	void Clear()
	{
		Lines.Empty();

		BorderColor = FNColor::GreyLight;
		BackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.75f);
		CurrentSeverity = ENSeverity::Message;

		Width = 0;
		Height = 0;
		bDirty = false;
	}
	
	/** Sets the current severity and updates the border color to match (Info/Message grey, Warning yellow, Error red, Fatal pink). */
	void SetSeverity(const ENSeverity Severity)
	{
		if (CurrentSeverity!=Severity)
		{
			CurrentSeverity = Severity;
		}
		
		switch (Severity)
		{
		case ENSeverity::Info:
			BorderColor = FNColor::GreyDark;
			break;
		case ENSeverity::Message:
			BorderColor = FNColor::GreyLight;
			break;
		case ENSeverity::Warning:
			BorderColor = FLinearColor::Yellow;
			break;
		case ENSeverity::Error:
			BorderColor = FLinearColor::Red;
			break;
		case ENSeverity::Fatal:
			BorderColor = FNColor::Pink;
			break;
		}
	}
	/** Raises the severity to the given level only when it is more severe than the current one. */
	void AddSeverity(const ENSeverity Severity)
	{
		if (Severity > CurrentSeverity)
		{
			SetSeverity(Severity);
		}
	}

	/** Appends a line rendered in the small font with the given color, marking the item dirty. */
	void AddSmallLine(const FText& Line, const FLinearColor Color = FLinearColor::White)
	{
		Lines.Add(FLine{ Line, Color, /* bIsLargeFont */ false });
		bDirty = true;
	}
	/** Appends a line rendered in the large font with the given color, marking the item dirty. */
	void AddLargeLine(const FText& Line, const FLinearColor Color = FLinearColor::White)
	{
		Lines.Add(FLine{ Line, Color, /* bIsLargeFont */ true });
		bDirty = true;
	}

	/** @return true if at least one line has been added. */
	bool HasContent() const
	{
		return !Lines.IsEmpty();
	}
	/** @return true if content has changed since the last layout pass and needs reprocessing before drawing. */
	bool IsDirty() const
	{
		return bDirty;
	}
	
private:

	/** One text line plus the layout measured for it by Process(). */
	struct FLine
	{
		// Content (set when the line is added).
		FText Text;
		FLinearColor Color = FLinearColor::White;
		bool bIsLargeFont = false;

		// Layout (computed by Process()).
		int32 Width = 0;
		int32 Height = 0;
		int32 Position = 0;
	};

	ENSeverity CurrentSeverity = ENSeverity::Message;

	FLinearColor BackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.75f);
	FLinearColor BorderColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.f);

	TArray<FLine> Lines;

	/** Overall box dimensions, computed by Process(). */
	int32 Width = 0;
	int32 Height = 0;

	void Process(int32 LineSpacing = 10)
	{
		UFont* SmallFont = GEngine->GetSmallFont();
		UFont* LargeFont = GEngine->GetLargeFont();

		Width = 0;
		Height = 0;
		int32 LinePositionOffset = LineSpacing;
		const int32 LineCount = Lines.Num();
		for (int32 i = 0; i < LineCount; ++i)
		{
			FLine& Line = Lines[i];

			int32 OutWidth;
			int32 OutHeight;
			const FString Text = Line.Text.ToString();
			if (Line.bIsLargeFont)
			{
				LargeFont->GetStringHeightAndWidth(Text, OutHeight, OutWidth);
			}
			else
			{
				SmallFont->GetStringHeightAndWidth(Text, OutHeight, OutWidth);
			}

			Line.Width = OutWidth;
			Line.Height = OutHeight;
			Line.Position = LinePositionOffset;

			Height += OutHeight;
			LinePositionOffset += (OutHeight + LineSpacing);
			if (OutWidth > Width)
			{
				Width = OutWidth;
			}

			if (i != LineCount - 1)
			{
				Height += LineSpacing;
			}
		}

		// No parallel arrays to validate — content and layout live together in FLine.
		bDirty = false;
	}

	bool bDirty = false;
};
