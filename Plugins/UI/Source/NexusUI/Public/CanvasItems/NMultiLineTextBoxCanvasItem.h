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
		Colors.Empty();
		bIsLargeFont.Empty();
		
		LineWidths.Empty();
		LineHeights.Empty();
		LinePositions.Empty();
		
		BorderColor = FNColor::GreyLight;
		BackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.75f);
		CurrentSeverity = ENSeverity::Message;
		
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
		Lines.Add(Line);
		Colors.Add(Color);
		bIsLargeFont.Add(false);
		bDirty = true;
	}
	/** Appends a line rendered in the large font with the given color, marking the item dirty. */
	void AddLargeLine(const FText& Line, const FLinearColor Color = FLinearColor::White)
	{
		Lines.Add(Line);
		Colors.Add(Color);
		bIsLargeFont.Add(true);
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
		
	ENSeverity CurrentSeverity = ENSeverity::Message;
	
	FLinearColor BackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.75f);
	FLinearColor BorderColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.f);
	
	TArray<FText> Lines;
	TArray<FLinearColor> Colors;
	TArray<bool> bIsLargeFont;
	
	TArray<int32> LineWidths;
	TArray<int32> LineHeights;
	TArray<int32> LinePositions;
	
	int32 Width;
	int32 Height;
	
	void Process(int32 LineSpacing = 10)
	{
		LineWidths.Empty();
		LineHeights.Empty();
		LinePositions.Empty();

		UFont* SmallFont = GEngine->GetSmallFont();
		UFont* LargeFont = GEngine->GetLargeFont();
		
		int32 OutWidth;
		int32 OutHeight;
		
		Width = 0;
		Height = 0;
		int32 LinePositionOffset = LineSpacing;
		const int LineCount = Lines.Num();
		for (int i = 0; i < LineCount; ++i)
		{
			const FString Line = Lines[i].ToString();
			if (bIsLargeFont[i])
			{
				LargeFont->GetStringHeightAndWidth(Line, OutHeight, OutWidth);
			}
			else
			{
				SmallFont->GetStringHeightAndWidth(Line, OutHeight, OutWidth);
				
			}
			LineWidths.Add(OutWidth);
			LineHeights.Add(OutHeight);
			LinePositions.Add(LinePositionOffset);
			
			Height += OutHeight;
			LinePositionOffset += (OutHeight + LineSpacing);
			if (OutWidth > Width)
			{
				Width = OutWidth;
			}
			
			if (i != Lines.Num() - 1)
			{
				Height += LineSpacing;
			}
		}

		// Validate
		
		if (Lines.Num() == LineCount &&  
			Colors.Num() == LineCount && 
			bIsLargeFont.Num() == LineCount && 
			LineWidths.Num() == LineCount && 
			LineHeights.Num() == LineCount && 
			LinePositions.Num() == LineCount)
		{
			bDirty = false;
		}
		else
		{
			
			UE_LOG(LogNexusUI, Warning, TEXT("Invalid MultiLineTextBoxCanvasItem contents, clearing and resetting."))
			Clear();
		}
	}
	
	bool bDirty = false;
};
