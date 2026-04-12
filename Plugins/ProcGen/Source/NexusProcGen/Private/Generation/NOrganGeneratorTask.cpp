// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

#include "NProcGenMinimal.h"
#include "Generation/NProcGenGraphBoneNode.h"
#include "Generation/NProcGenGraphCellNode.h"
#include "Generation/NProcGenGraphNodeFactory.h"
#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Context(ContextPtr.ToSharedRef()), PassContext(PassContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{
}

// TODO: Spin up multiple of these tasks? with different sub-seeds? Maybe we make a seperate task under this

// REMINDERS
// - Need a quick way to make sure that a spawned cell is INSIDE the volume


void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// The context was not validated during creation, so we cannot process it
	if (!Context->IsValid()) return;
	
	// Create our deterministic random for the task which will get passed byref to sub-methods
	FNMersenneTwister Random(Context->Seed);
	
	// Find the bone and build our starting cell
	StartGraph(Random);
	
	int NodeCount = Context->CellGraph->GetNodeCount();
	
	// TEMP TEST: Process our starting node
	TArray<FNProcGenGraphNode*> NewNodes = ProcessNode(Random, Context->CellGraph->GetLastNode());
	
	
	Context->bSuccessful = true;
}

void FNOrganGeneratorTask::StartGraph(FNMersenneTwister& Random) const
{
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNBoneInputData& BoneData = Context->BoneInputData[0];
	
	// Get a weighted array of indices representing possible starting cells, and reference the cell data
	FNWeightedIntegerArray WeightedStartIndices = Context->GenerateWeightedStartCellIndices(BoneData.SocketSize);
	const int32 StartCellIndex = WeightedStartIndices.TwistedValue(Random);
	FNCellInputData StartCellInputData = Context->CellInputData[StartCellIndex];
	
	// Decide which appropriately sized junction is going to be used
	TArray<int32> StartCellJunctionKeys = StartCellInputData.GetJunctionKeys(BoneData.SocketSize);
	const int StartCellJunctionKeyIndex = Random.IntegerRange(0, StartCellJunctionKeys.Num()-1); // need to flag this later
	const FNCellJunctionDetails* StartCellJunctionDetails = StartCellInputData.Junctions.Find(StartCellJunctionKeys[StartCellJunctionKeyIndex]);
	
	// Figure out rotation
	const FQuat BoneQuat = BoneData.WorldRotation.Quaternion();
	const FQuat JunctionQuat = StartCellJunctionDetails->WorldRotation.Quaternion();
	
	// TODO : HANDLE IDENTITY! x0
	const FQuat CellWorldQuat = BoneQuat * JunctionQuat.Inverse();
	const FRotator CellWorldRotation = CellWorldQuat.Rotator();
	const FVector JunctionWorldOffset = CellWorldQuat.RotateVector(StartCellJunctionDetails->WorldLocation);
	const FVector CellWorldPosition = BoneData.WorldPosition - JunctionWorldOffset;
	
	// Create our bone node and build a graph around it.
	FNProcGenGraphBoneNode* BoneNode = FNProcGenGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph = MakeUnique<FNProcGenGraph>(BoneNode);
	
	// Create our first cell node, attaching it to the bone node
	FNProcGenGraphCellNode* StartNode = FNProcGenGraphNodeFactory::CreateCellNode(&StartCellInputData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph->RegisterNode(StartNode);
	
	// Link our nodes
	BoneNode->Link(StartNode);
	StartNode->Link(StartCellJunctionKeys[StartCellJunctionKeyIndex], BoneNode);
}

TArray<FNProcGenGraphNode*> FNOrganGeneratorTask::ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const
{
	switch (SourceNode->GetNodeType())
	{
		using enum ENProcGenGraphNodeType;
	case Cell:
		return ProcessCellNode(Random, static_cast<FNProcGenGraphCellNode*>(SourceNode));
	case Bone:
	case Null:
	default:
		break;
	}
	return TArray<FNProcGenGraphNode*>();
}

TArray<FNProcGenGraphNode*> FNOrganGeneratorTask::ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const
{
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = SourceCellNode->GetOpenJunctions();
	TArray<FNProcGenGraphNode*> NewNodes;
	
	for (const auto Junction : OpenJunctions)
	{
		// Build our possible list of cells
		FNWeightedIntegerArray CellInputWeightedIndices = Context->GenerateWeightedCellInputIndices(Junction.Value->SocketSize);
		
		// We don't have any cell input data able to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.Count() == 0)
		{
			// TODO: We will later go back and fill this with something.
			FNProcGenGraphNullNode* NullNode = FNProcGenGraphNodeFactory::CreateNullNode(Junction.Value->WorldLocation, Junction.Value->WorldRotation);
			Context->CellGraph->RegisterNode(NullNode);
			
			SourceCellNode->Link(Junction.Key, NullNode);
			NullNode->Link(SourceCellNode);
			continue;
		}
		
		// Pick our cell to use to spawn
		const int32 CellInputIndex = CellInputWeightedIndices.TwistedValue(Random);
		FNCellInputData CellInputData = Context->CellInputData[CellInputIndex];
		
		// Pick the junction of the cell we are going to use
		TArray<int32> TargetJunctionKeys = CellInputData.GetJunctionKeys(Junction.Value->SocketSize);
		const int TargetJunctionKeyIndex = Random.IntegerRange(0, TargetJunctionKeys.Num()-1);
		const int TargetJunctionKey = TargetJunctionKeys[TargetJunctionKeyIndex];
		const FNCellJunctionDetails* TargetJunctionDetails = CellInputData.Junctions.Find(TargetJunctionKey);
	
		// Figure out rotations
		FQuat SourceJunctionWorldQuat = Junction.Value->WorldRotation.Quaternion();
		FQuat TargetJunctionLocalQuat = TargetJunctionDetails->WorldRotation.Quaternion();
		
		
		// TODO : HANDLE IDENTITY! x0
		FQuat RequiredRotationQuat;
		if (TargetJunctionLocalQuat.IsIdentity())
		{
			UE_LOG(LogNexusProcGen, Error, TEXT("IDENTITY ROTATION %i"), TargetJunctionKey);
			RequiredRotationQuat = TargetJunctionLocalQuat * SourceJunctionWorldQuat;
		}
		else if(SourceJunctionWorldQuat.IsIdentity())
		{
			UE_LOG(LogNexusProcGen, Error, TEXT("IDENTITY ROTATION %i"), TargetJunctionKey);
			RequiredRotationQuat = TargetJunctionLocalQuat * SourceJunctionWorldQuat;
		}
		else
		{
			RequiredRotationQuat = TargetJunctionLocalQuat * SourceJunctionWorldQuat;
		}
		
		FRotator RequiredRotation = RequiredRotationQuat.Rotator(); 
		
		FVector TargetJunctionWorldOffset = RequiredRotationQuat.RotateVector(TargetJunctionDetails->WorldLocation);
		FVector TargetJunctionWorldPosition = Junction.Value->WorldLocation - TargetJunctionWorldOffset;
	
		FNProcGenGraphCellNode* TargetCellNode = FNProcGenGraphNodeFactory::CreateCellNode(&CellInputData, TargetJunctionWorldPosition, RequiredRotation);
		Context->CellGraph->RegisterNode(TargetCellNode);
		
		SourceCellNode->Link(Junction.Key, TargetCellNode);
		TargetCellNode->Link(TargetJunctionKey, SourceCellNode);
		
		NewNodes.Add(TargetCellNode);
	}
	
	return MoveTemp(NewNodes);
}
