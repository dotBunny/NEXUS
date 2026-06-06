// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NAssemblyFinalizeTask.h"
#include "Assembly/NAssemblyOperation.h"
#include "Containers/Ticker.h"


FNAssemblyFinalizeTask::FNAssemblyFinalizeTask(UNAssemblyOperation* TargetOperation, 
                                                         const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: Operation(TargetOperation), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
	
}

void FNAssemblyFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	if (!IsValid(Operation))
	{
		return;
	}
	
#if !UE_BUILD_SHIPPING
	
	
	const int32 TaggingContentTicket = Operation->GetReport()->CreateContentBlock();
	FNReportContentBlock* TaggingContentBlock = Operation->GetReport()->GetContentBlock(TaggingContentTicket);
	TaggingContentBlock->SetHeading("Tagging");
	
	// Add our Context Tags to report
	const int32 ContextTagsContentTicket = Operation->GetReport()->CreateContentBlock(TaggingContentTicket);
	FNReportContentBlock* ContextTagsContentBlock = Operation->GetReport()->GetContentBlock(ContextTagsContentTicket);
	ContextTagsContentBlock->SetHeading("Context Tags");
	
	FStringBuilderBase StringBuilder;
	for (const FGameplayTag& Tag : TaskGraphContextPtr->ContextTags)
	{
		StringBuilder.Append(Tag.ToString());
		StringBuilder.Append(", ");
	}
	if (StringBuilder.Len() > 2)
	{
		StringBuilder.RemoveSuffix(2);
	}
	ContextTagsContentBlock->AddLine(StringBuilder.ToString());
	
	// Add our Tag Counter to report
	const int32 TagCounterContentTicket = Operation->GetReport()->CreateTableBlock(TaggingContentTicket);
	FNReportTableBlock* TagCounterTableBlock = Operation->GetReport()->GetTableBlock(TagCounterContentTicket);
	TagCounterTableBlock->SetHeading("Tag Counter");
	TagCounterTableBlock->Initialize({ "Tag", "Count"});
	for (const TPair<FGameplayTag, int32>& Pair : TaskGraphContextPtr->TagCounter.GameplayTags)
	{
		TagCounterTableBlock->AddRow({ Pair.Key.ToString(), FString::FromInt(Pair.Value) });
	}

	// Add Analytics to report
	N_ASSEMBLY_ANALYTICS_MEMBER_PTR->AddToReports(Operation->GetReport(), Operation->GetDetailedReports());
	TaskGraphContextPtr->ReportFilePath = Operation->OutputReportsToFile();
#endif // !UE_BUILD_SHIPPING

	// Send the finalized shared data back to the operation for doings.
	// Defer to next tick so FinishBuild never runs inside a named-thread pump triggered by
	// a sync load / FlushRenderingCommands — ProcessEvent (e.g. relay Client_* RPCs) would
	// assert if a PostLoad is still on the stack above us.
	TWeakObjectPtr<UNAssemblyOperation> WeakOperation(Operation);
	TSharedRef<FNAssemblyTaskGraphContext> ContextRef = TaskGraphContextPtr;
	FTSTicker::GetCoreTicker().AddTicker(TEXT("NAssemblyFinalize"), 0.0f,
		[WeakOperation, ContextRef](float) -> bool
		{
			if (UNAssemblyOperation* Op = WeakOperation.Get(); IsValid(Op))
			{
				Op->FinishBuild(ContextRef);
				// The operation at this point has been flagged as garbage, don't use it.
			}
			return false;
		});
}
