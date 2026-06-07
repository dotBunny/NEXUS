// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/NReportBlock.h"


struct NEXUSCORE_API FNReportListBlock : FNReportBlock
{
	explicit FNReportListBlock(const int32 Ticket)
	: FNReportBlock(Ticket)
	{
	}

	void AddItem(const FString& Item);


	virtual void Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) override;
private:
	TArray<FString> Items;
	
	void RenderContentAsMarkdown(FNReport& Report, TArray<FString>& Output);
	
	void RenderContentAsPlainText(FNReport& Report, TArray<FString>& Output);
};
