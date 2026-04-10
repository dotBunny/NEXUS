// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorFinalizeTask.h"

#include "NProcGenOperation.h"
#include "Cell/NCellProxy.h"


FNOrganGeneratorFinalizeTask::FNOrganGeneratorFinalizeTask(
	UNProcGenOperation* TargetOperation, const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
		: Operation(TargetOperation), Context(ContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{

}

void FNOrganGeneratorFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	if (!Context->IsSuccessful())
	{
		return;
	}
	
	// NOT SURE IF THIS IS WHERE WE WANT TO DO THE BUILD, BUT FOR NOW LETS GO
	for (auto Cell : Context->CellOutputData)
	{
		// Spawn proxy instance
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(SharedContext->TargetWorld, Cell.Template, Cell.WorldPosition, Cell.WorldRotation, false);
		
		// Registered with global?
		SharedContext->CreatedProxies.Add(Proxy);
	}
	
	
	// Create objects from context?
	// Write details to pass context?
	// Write to shared?
	
	
//	FPlatformProcess::Sleep(5.0f); 
// Dont sleep on main thread K thnx	
	
//	Operation->FinishOrgan();
}
