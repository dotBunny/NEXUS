// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/Report/NReportTableBlock.h"

#include "NCoreMinimal.h"
#include "Developer/Report/NReport.h"

void FNReportTableBlock::Initialize(const int32 Columns)
{
	ColumnCount = Columns;
	MaximumCellLengths.Empty();
	HeaderCells.Empty();
	Cells.Empty();
	
	for (int i = 0; i < ColumnCount; i++)
	{
		MaximumCellLengths.Add(0);
	}
}

void FNReportTableBlock::Initialize(const TArray<FString>& HeaderRow)
{
	ColumnCount = HeaderRow.Num();
	HeaderCells.Empty();
	Cells.Empty();
	
	for (int i = 0; i < ColumnCount; i++)
	{
		HeaderCells.Add(HeaderRow[i]);
		MaximumCellLengths.Add(HeaderRow[i].Len());
	}
}

void FNReportTableBlock::AddRow(const TArray<FString>& Row)
{
	if (ColumnCount == -1)
	{
		UE_LOG(LogNexusCore, Error, TEXT("Unable to add data row as the FNReportTableBlock has not been initialized yet."));
	}
	
	int Columns = Row.Num();
	if (Columns > ColumnCount)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Attempting to add data row exceeding number(%i) of initialized columns(%i), additional data will be discarded."), Row.Num(), ColumnCount);
		Columns = ColumnCount;
	}
	
	
	TArray<FString>& NewRow = Cells.Add_GetRef(TArray<FString>());
	for (int i = 0; i < Columns; i++)
	{
		NewRow.Add(Row[i]);
		const int32 Length = Row[i].Len();
		if (MaximumCellLengths[i] < Length)
		{
			MaximumCellLengths[i] = Length;
		}
	}
}

void FNReportTableBlock::Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	RenderHeading(Output, OutputFormat);
	RenderHeader(Output, OutputFormat);
	

	// Build Table
	FStringBuilderBase Builder;
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		Builder.Append(TEXT("|"));
		for (int32 i = 0; i < ColumnCount; i++)
		{
			Builder.Append(TEXT(" "));
			Builder.Append(HeaderCells.IsValidIndex(i) ? HeaderCells[i] : FString());
			Builder.Append(TEXT(" |"));
		}
		Output.Add(Builder.ToString());

		Builder.Reset();
		Builder.Append(TEXT("|"));
		for (int32 i = 0; i < ColumnCount; i++)
		{
			Builder.Append(TEXT(" --- |"));
		}
		Output.Add(Builder.ToString());

		for (int32 r = 0; r < Cells.Num(); r++)
		{
			Builder.Reset();
			Builder.Append(TEXT("|"));
			const TArray<FString>& Row = Cells[r];
			for (int32 c = 0; c < ColumnCount; c++)
			{
				Builder.Append(TEXT(" "));
				Builder.Append(Row.IsValidIndex(c) ? Row[c] : FString());
				Builder.Append(TEXT(" |"));
			}
			Output.Add(Builder.ToString());
		}
	}
	else
	{
		if (HeaderCells.Num() > 0)
		{
			for (int32 i = 0; i < ColumnCount; i++)
			{
				if (i > 0)
				{
					Builder.Append(TEXT("  "));
				}
				const FString& Cell = HeaderCells[i];
				Builder.Append(Cell);
				for (int32 p = Cell.Len(); p < MaximumCellLengths[i]; p++)
				{
					Builder.Append(TEXT(" "));
				}
			}
			Output.Add(Builder.ToString());

			Builder.Reset();
			for (int32 i = 0; i < ColumnCount; i++)
			{
				if (i > 0)
				{
					Builder.Append(TEXT("  "));
				}
				for (int32 p = 0; p < MaximumCellLengths[i]; p++)
				{
					Builder.Append(TEXT("-"));
				}
			}
			Output.Add(Builder.ToString());
		}

		for (int32 r = 0; r < Cells.Num(); r++)
		{
			Builder.Reset();
			const TArray<FString>& Row = Cells[r];
			for (int32 c = 0; c < ColumnCount; c++)
			{
				if (c > 0)
				{
					Builder.Append(TEXT("  "));
				}
				const FString Cell = Row.IsValidIndex(c) ? Row[c] : FString();
				Builder.Append(Cell);
				for (int32 p = Cell.Len(); p < MaximumCellLengths[c]; p++)
				{
					Builder.Append(TEXT(" "));
				}
			}
			Output.Add(Builder.ToString());
		}
	}
	
	// Children
	TArray<int32> ChildrenTickets;
	Report.GetOrderedBlocks(GetTicket(), ChildrenTickets, false);
	
	// Children
	for (int32 i = 0; i < ChildrenTickets.Num(); i++)
	{
		Report.RenderBlock(ChildrenTickets[i], Output, OutputFormat);
	}
	
	RenderFooter(Output, OutputFormat);
}
