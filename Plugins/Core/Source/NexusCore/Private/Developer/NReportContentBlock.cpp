// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReportContentBlock.h"
#include "Developer/NReport.h"

void FNReportContentBlock::Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	RenderHeading(Output, OutputFormat);
	RenderHeader(Output, OutputFormat);
	
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		for (int32 i = 0; i < Content.Num(); i++)
		{
			Output.Add(Content[i]);
			Output.Add(TEXT(""));
		}
	}
	else
	{
		for (int32 i = 0; i < Content.Num(); i++)
		{
			Output.Add(Content[i]);
			Output.Add(TEXT(""));
		}
	}
	
	TArray<int32> ChildrenTickets;
	Report.GetOrderedBlocks(GetTicket(), ChildrenTickets, false);
	
	// Children
	for (int32 i = 0; i < ChildrenTickets.Num(); i++)
	{
		Report.RenderBlock(ChildrenTickets[i], Output, OutputFormat);
	}
	
	
	
	RenderFooter(Output, OutputFormat);
}
