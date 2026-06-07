// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReportCollapsableBlock.h"
#include "Developer/NReport.h"

void FNReportCollapsableBlock::Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	TArray<int32> ChildrenTickets;
	Report.GetOrderedBlocks(GetTicket(), ChildrenTickets, false);

	// Collapse: with no child blocks this section contributes nothing — not even its heading/header/footer.
	if (ChildrenTickets.Num() == 0)
	{
		return;
	}

	RenderHeading(Output, OutputFormat);
	RenderHeader(Output, OutputFormat);

	for (int32 i = 0; i < ChildrenTickets.Num(); i++)
	{
		Report.RenderBlock(ChildrenTickets[i], Output, OutputFormat);
	}

	RenderFooter(Output, OutputFormat);
}
