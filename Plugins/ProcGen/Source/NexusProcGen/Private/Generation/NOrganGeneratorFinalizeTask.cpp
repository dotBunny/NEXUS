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
	
	const FNProcGenOperationSettings& Settings = SharedContext->OperationSettings;
	
	// NOT SURE IF THIS IS WHERE WE WANT TO DO THE BUILD, BUT FOR NOW LETS GO
	for (const auto Node : Context->CellGraph->GetNodes())
	{
		if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
		{
			FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
			
			// TODO: This is where we need to decide on what replication is going to do. 
			// - Replication right now seems to Replicate the LevelInstance around. It doesnt have much local context, but 
			//	 it follows through with what the server does.
			// - One option would be to try to replicate the junction data on the LevelInstance data, but that doesnt solve the PCG graphs that look
			//   for junction information. Would need to add something to inject the remote IF pcg isnt networked?

			
			// Spawn proxy instance
			ANCellProxy* Proxy = ANCellProxy::CreateInstance(SharedContext->TargetWorld, CellNode, Settings.bPreLoadLevelInstances);
		
			// Registered with global?
			SharedContext->CreatedProxies.Add(Proxy);
			
			
			// What about creating the instance?
			if  (Settings.bCreateLevelInstances)
			{
				Proxy->CreateLevelInstance();
			}
			
			// Do we want to load these now?
			if (Settings.bLoadLevelInstances)
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

