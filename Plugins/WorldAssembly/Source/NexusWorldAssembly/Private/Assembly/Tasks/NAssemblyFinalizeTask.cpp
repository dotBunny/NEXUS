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
	
	// Add our Output Tags to report
	const int32 OutputTagsContentTicket = Operation->GetReport()->CreateContentBlock();
	FNReportContentBlock* OutputTagsContentBlock = Operation->GetReport()->GetContentBlock(OutputTagsContentTicket);
	OutputTagsContentBlock->SetHeading("Output Tags");
	
	FStringBuilderBase StringBuilder;
	for (const FGameplayTag& Tag : TaskGraphContextPtr->OutputTags)
	{
		StringBuilder.Append(Tag.ToString());
		StringBuilder.Append(", ");
	}
	if (StringBuilder.Len() > 2)
	{
		StringBuilder.RemoveSuffix(2);
	}
	OutputTagsContentBlock->AddLine(StringBuilder.ToString());
	
	
	// Add Analytics to report
	N_ASSEMBLY_ANALYTICS_MEMBER_PTR->AddToReport(Operation->GetReport());
	TaskGraphContextPtr->ReportFilePath = Operation->OutputReportToFile();
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
