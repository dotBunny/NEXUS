// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"

void FNReportContentBlock::AddContentBlock(const FNReportContentBlock& ChildContentBlock)
{
	int32 InsertIndex = Children.Num();
	for (int32 i = 0; i < Children.Num(); i++)
	{
		if (Children[i].Priority > ChildContentBlock.Priority)
		{
			InsertIndex = i;
			break;
		}
	}
	
	Children.Insert(ChildContentBlock, InsertIndex);
}


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
	return Children.Last();
}

FNReportContentBlock& FNReportContentBlock::CreateContentBlock(const FString& ContentHeading, const int OrderPriority)
{
	Children.Add(FNReportContentBlock());
	
	FNReportContentBlock& ContentBlock = Children.Last();
	ContentBlock.Priority = OrderPriority;
	ContentBlock.Heading = ContentHeading;
	
	
	return ContentBlock;
}
