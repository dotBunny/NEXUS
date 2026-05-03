// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/Report/NReportBlock.h"

/**
 * Report block that holds tabular data. Must be initialized with either a fixed column count
 * or a header row before rows are added. Plain-text rendering left-aligns each cell to the
 * widest entry in its column; Markdown rendering emits a standard pipe-separated table.
 */
struct NEXUSCORE_API FNReportTableBlock : FNReportBlock
{
	/** Construct a table block with the ticket issued by the owning FNReport; not intended for direct use. */
	explicit FNReportTableBlock(const int32 Ticket)
	: FNReportBlock(Ticket)
	{
	}

	/**
	 * Initialize the table with a fixed number of columns and no header row.
	 * @param Columns Number of columns the table will have; rows exceeding this are truncated.
	 */
	void Initialize(const int32 Columns);
	/**
	 * Initialize the table from a header row, deriving the column count and seeding column widths.
	 * @param HeaderRow Header cells, one per column.
	 */
	void Initialize(const TArray<FString>& HeaderRow);

	/**
	 * Append a row of cells, updating per-column maximum widths as needed.
	 * @param Row Cell values for this row; trailing cells beyond the column count are dropped.
	 * @note Initialize must have been called first; otherwise the row is rejected with a log error.
	 */
	void AddRow(const TArray<FString>& Row);

	/**
	 * Render this table (heading, header, table body, child blocks, footer) into Output.
	 * @param Report The owning report, used to resolve and render this block's children.
	 * @param Output Line buffer that this block appends to.
	 * @param OutputFormat Whether to emit plain text (column-aligned) or Markdown (pipe-separated).
	 */
	virtual void Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) override;

private:
	/** Header row cells; empty when the table was initialized with a column count rather than a header row. */
	TArray<FString> HeaderCells;
	/** Body rows, each storing one cell value per column. */
	TArray<TArray<FString>> Cells;
	/** Widest cell length seen per column, used to drive plain-text padding. */
	TArray<int32> MaximumCellLengths;

	/** Number of columns the table was initialized with; -1 indicates the table has not been initialized yet. */
	int32 ColumnCount = -1;
};
