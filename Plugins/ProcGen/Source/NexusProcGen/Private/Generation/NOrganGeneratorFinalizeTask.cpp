// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorFinalizeTask.h"

#include "NProcGenOperation.h"
#include "Cell/NCellProxy.h"
#include "Developer/NDebugActor.h"
#include "Generation/NProcGenGraphCellNode.h"


FNOrganGeneratorFinalizeTask::FNOrganGeneratorFinalizeTask(
	UNProcGenOperation* TargetOperation, const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
		: Operation(TargetOperation), Context(ContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{

}

void FNOrganGeneratorFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	if (!Context->IsSuccessful() || Context->CellGraph == nullptr)
	{
		return;
	}
	
	// NOT SURE IF THIS IS WHERE WE WANT TO DO THE BUILD, BUT FOR NOW LETS GO
	for (const auto Node : Context->CellGraph->GetNodes())
	{
		if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
		{
			FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
			
			// Spawn proxy instance
			ANCellProxy* Proxy = ANCellProxy::CreateInstance(SharedContext->TargetWorld, CellNode, false);
		
			// Registered with global?
			SharedContext->CreatedProxies.Add(Proxy);
			
			
			
			// What about creating the instance?
			if  (SharedContext->bCreateLevelInstances)
			{
				Proxy->CreateLevelInstance();
			}
			
			// Do we want to load these now?
			if (SharedContext->bLoadLevelInstances)
			{
				Proxy->LoadLevelInstance();
			}
		}
	}
	
	
	// Create objects from context?
	// Write details to pass context?
	// Write to shared?
	
	
//	FPlatformProcess::Sleep(5.0f); 
// Dont sleep on main thread K thnx	
	
//	Operation->FinishOrgan();
}

