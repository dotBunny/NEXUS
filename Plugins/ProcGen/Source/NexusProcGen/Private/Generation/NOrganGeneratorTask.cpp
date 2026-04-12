// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

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
	ProcessNode(Random, Context->CellGraph->GetLastNode());
	
	
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
	const FQuat JunctionQuat = StartCellJunctionDetails->RootRelativeRotation.Quaternion();
	const FQuat CellWorldQuat = BoneQuat * JunctionQuat.Inverse();
	const FRotator CellWorldRotation = CellWorldQuat.Rotator();
	const FVector JunctionWorldOffset = CellWorldQuat.RotateVector(StartCellJunctionDetails->RootRelativeLocation);
	const FVector CellWorldPosition = BoneData.WorldPosition - JunctionWorldOffset;
	
	// Create our bone node and build a graph around it.
	FNProcGenGraphBoneNode* BoneNode = FNProcGenGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph = MakeUnique<FNProcGenGraph>(BoneNode);
	
	// Create our first cell node, attaching it to the bone node
	FNProcGenGraphCellNode* StartNode = FNProcGenGraphNodeFactory::CreateCellNode(&StartCellInputData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph->RegisterNode(StartNode);
	
	BoneNode->Link(StartNode);
	StartNode->Link(StartCellJunctionKeys[StartCellJunctionKeyIndex], BoneNode);
	
	// TODO: StartNode->ApplyJunctionsOffset()
}

bool FNOrganGeneratorTask::ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const
{
	switch (SourceNode->GetNodeType())
	{
		using enum ENProcGenGraphNodeType;
	case Cell:
		return ProcessCellNode(Random, static_cast<FNProcGenGraphCellNode*>(SourceNode));
	case Bone:
		return false;
	case Null:
		return false;
	}
	return false;
}

bool FNOrganGeneratorTask::ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const
{
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = SourceCellNode->GetOpenJunctions();
	int AddedNodeCount = 0;
	
	for (const auto Junction : OpenJunctions)
	{
		// Build our possible list of cells
		FNWeightedIntegerArray CellInputWeightedIndices = Context->GenerateWeightedCellInputIndices(Junction.Value->SocketSize);
		
		// We don't have any cell input data able to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.Count() == 0)
		{
			// TODO: We will later go back and fill this with something.
			FNProcGenGraphNullNode* NullNode = FNProcGenGraphNodeFactory::CreateNullNode(Junction.Value->RootRelativeLocation, Junction.Value->RootRelativeRotation);
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
		FQuat SourceJunctionQuat = Junction.Value->RootRelativeRotation.Quaternion();
		FQuat TargetJunctionQuat = TargetJunctionDetails->RootRelativeRotation.Quaternion();
		FQuat TargetJunctionWorldQuat = SourceJunctionQuat * TargetJunctionQuat.Inverse();
		FRotator TargetJunctionWorldRotation = TargetJunctionWorldQuat.Rotator(); 
		FVector TargetJunctionWorldOffset = TargetJunctionWorldQuat.RotateVector(Junction.Value->RootRelativeLocation);
		FVector TargetJunctionWorldPosition = Junction.Value->RootRelativeLocation - TargetJunctionWorldOffset;
	
		FNProcGenGraphCellNode* TargetCellNode = FNProcGenGraphNodeFactory::CreateCellNode(&CellInputData, TargetJunctionWorldPosition, TargetJunctionWorldRotation);
		Context->CellGraph->RegisterNode(TargetCellNode);
		
		SourceCellNode->Link(Junction.Key, TargetCellNode);
		TargetCellNode->Link(TargetJunctionKey, SourceCellNode);
		
		AddedNodeCount++;
		// TODO: Calculate offset
		//TargetCellNode->ApplyJunctionsOffset()
	}
	
	return AddedNodeCount > 0;
}
