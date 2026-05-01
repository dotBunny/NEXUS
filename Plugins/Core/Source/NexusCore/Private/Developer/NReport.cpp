// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"
uint64 FNReportContentBlock::ContentBlockTickets = 0;

void FNReportContentBlock::RemoveContentBlock(const FNReportContentBlock& ChildContentBlock)
{
	if (Children.Contains(ChildContentBlock))
	{
		Children.Remove(ChildContentBlock);
	}
}

FNReportContentBlock& FNReportContentBlock::CreateContentBlock()
{
	Children.Add(FNReportContentBlock());
	
	FNReportContentBlock& ContentBlock = Children.Last();
	ContentBlock.Level = Level + 1;
	return Children.Last();
}

FNReportContentBlock& FNReportContentBlock::CreateContentBlock(const FString& ContentHeading, const int OrderPriority)
{
	int32 InsertIndex = Children.Num();
	for (int32 i = 0; i < Children.Num(); i++)
	{
		if (Children[i].Priority > OrderPriority)
		{
			InsertIndex = i;
			break;
		}
	}
	
	Children.Insert(FNReportContentBlock(), InsertIndex);
	
	FNReportContentBlock& ContentBlock = Children[InsertIndex];
	
	ContentBlock.Priority = OrderPriority;
	ContentBlock.Heading = ContentHeading;
	ContentBlock.Level = Level + 1;
	
	return ContentBlock;
}
