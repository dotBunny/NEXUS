// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReportListBlock.h"
#include "Developer/NReport.h"

void FNReportListBlock::AddItem(const FString& Item)
{
	Items.Add(Item);
}

void FNReportListBlock::Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	RenderHeading(Output, OutputFormat);
	RenderHeader(Output, OutputFormat);

	// Build Table
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		RenderContentAsMarkdown(Report, Output);
	}
	else
	{
		RenderContentAsPlainText(Report, Output);
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

// #SONARQUBE-DISABLE-CPP_S3776 It just is what it is to handle edge cases

void FNReportListBlock::RenderContentAsMarkdown(FNReport& Report, TArray<FString>& Output)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		Output.Add(FString::Printf(TEXT("- %s"), *Items[i]));
	}
}

void FNReportListBlock::RenderContentAsPlainText(FNReport& Report, TArray<FString>& Output)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		Output.Add(FString::Printf(TEXT("- %s"), *Items[i]));
	}
}

// #SONARQUBE-ENABLE-CPP_S3776 It just is what it is to handle edge cases
