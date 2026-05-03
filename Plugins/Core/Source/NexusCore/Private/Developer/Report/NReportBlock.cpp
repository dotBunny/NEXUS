// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/Report/NReportBlock.h"

#include "Developer/Report/NReport.h"

void FNReportBlock::Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	RenderHeading(Output, OutputFormat);
	RenderHeader(Output, OutputFormat);
	
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

void FNReportBlock::RenderHeading(TArray<FString>& Output, const ENReportOutputFormat OutputFormat) const
{
	if (HeadingLength <= 0) return;
	
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		switch (Level)
		{
			case 1:
				Output.Add(TEXT("# ") + Heading);
				return;
			case 2:
				Output.Add(TEXT("## ") + Heading);
				return;
			case 3:
				Output.Add(TEXT("### ") + Heading);
				return;
			case 4:
				Output.Add(TEXT("#### ") + Heading);
				return;
			case 5:
				Output.Add(TEXT("##### ") + Heading);
				return;			
			default:
				Output.Add(Heading);
				return;
		}
	}
	
	FStringBuilderBase Builder;
	Output.Add(TEXT(""));
	switch (Level)
	{
	case 1:
		for (int i = 0; i < HeadingLength; i++)
		{
			Builder.Append("*");
		}
		Output.Add(Heading);
		Output.Add(Builder.ToString());
		return;
	case 2:
		for (int i = 0; i < HeadingLength; i++)
		{
			Builder.Append("=");
		}
		Output.Add(Heading);
		Output.Add(Builder.ToString());
		return;
	case 3:
		for (int i = 0; i < HeadingLength; i++)
		{
			Builder.Append("-");
		}
		Output.Add(Heading);
		Output.Add(Builder.ToString());
		return;
	case 4:
		Output.Add("***" + Heading + "***");
		return;
	case 5:
		Output.Add("===" + Heading + "===");
		return;			
	default:
		Output.Add("---" + Heading + "---");
	}
}

void FNReportBlock::RenderHeader(TArray<FString>& Output, const ENReportOutputFormat OutputFormat) const
{
	if (Header.IsEmpty()) return;
	
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		Output.Add("> " + Header);
		return;
	}
	Output.Add(Header);
}

void FNReportBlock::RenderFooter(TArray<FString>& Output, const ENReportOutputFormat OutputFormat) const
{
	if (Footer.IsEmpty()) return;
	
	if (OutputFormat == ENReportOutputFormat::Markdown)
	{
		Output.Add("> " + Footer);
		return;
	}
	Output.Add(Footer);
}
