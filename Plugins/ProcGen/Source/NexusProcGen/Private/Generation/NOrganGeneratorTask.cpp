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

void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// The context was not validated during creation, so we cannot process it
	if (!Context->IsValid()) return;
	
	// Create our deterministic random for the task
	FNMersenneTwister Random(Context->Seed);
	
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNBoneInputData& BoneData = Context->BoneInputData[0];
	
	// TODO: These Generate functions of indices should also take into account rotation of the socket, and the rotational limits on the cell settings
	
	// Get a weighted array of indices representing possible starting cells, and reference the cell data
	FNWeightedIntegerArray WeightedStartIndices = Context->GenerateWeightedStartCellIndices(BoneData.SocketSize);
	int32 StartCellIndex = WeightedStartIndices.TwistedValue(Random);
	FNCellInputData StartCellInputData = Context->CellInputData[StartCellIndex];
	
	// Decide which appropriately sized junction is going to be used
	// TODO: later this will need to check if filled when used again in the later (ie need tracking)
	TArray<int32> StartCellJunctionKeys = StartCellInputData.GetJunctionKeys(BoneData.SocketSize);
	const int StartCellJunctionKeyIndex = Random.IntegerRange(0, StartCellJunctionKeys.Num()-1); // need to flag this later
	FNCellJunctionDetails* StartCellJunctionDetails = StartCellInputData.Junctions.Find(StartCellJunctionKeys[StartCellJunctionKeyIndex]);
	
	
	// START figure out rotation
	
	
	
	
	// Establish a base understanding of the rotations/directions

	FQuat BoneQuat = BoneData.WorldRotation.Quaternion();
	FQuat JunctionQuat = StartCellJunctionDetails->RootRelativeRotation.Quaternion();

	// The cell's world rotation = BoneRotation * Inverse(JunctionRelativeRotation)
	// This makes the junction align with the bone socket
	FQuat CellWorldQuat = BoneQuat * JunctionQuat.Inverse();
	FRotator CellWorldRotation = CellWorldQuat.Rotator();

	// Step 3: Calculate the cell's world position
	// Transform the junction's relative location by the cell's world rotation, then offset from bone position
	FVector JunctionWorldOffset = CellWorldQuat.RotateVector(StartCellJunctionDetails->RootRelativeLocation);
	FVector CellWorldPosition = BoneData.WorldPosition - JunctionWorldOffset;
	
	// END figure out rotation
	
	
	// Create our bone node and build a graph around it.
	FNProcGenGraphBoneNode* BoneNode = FNProcGenGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph = MakeUnique<FNProcGenGraph>(BoneNode);
	
	// Create our first cell node, attaching it to the bone node
	FNProcGenGraphCellNode* StartNode = FNProcGenGraphNodeFactory::CreateCellNode(&StartCellInputData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph->RegisterNode(StartNode);
	
	BoneNode->Link(StartNode);
	StartNode->Link(StartCellJunctionKeys[StartCellJunctionKeyIndex], BoneNode);
	
	int NodeCount = Context->CellGraph->GetNodeCount();
	
	
	// --- THIS IS GONNA BE A METHOD? -- this is just a test of building off the first spawn
	
	FNProcGenGraphCellNode* TargetNode =  StartNode;
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = TargetNode->GetOpenJunctions();
	
	for (const auto Junction : OpenJunctions)
	{
		// Build our possible list of cells
		FNWeightedIntegerArray CellInputWeightedIndices = Context->GenerateWeightedCellInputIndices(Junction.Value->SocketSize);
		
		// We dont have any cell input data able to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.Count() == 0)
		{
			// TODO: We will later go back and fill this with something.
			FNProcGenGraphNullNode* NullNode = FNProcGenGraphNodeFactory::CreateNullNode(Junction.Value->RootRelativeLocation, Junction.Value->RootRelativeRotation);
			Context->CellGraph->RegisterNode(NullNode);
			
			TargetNode->Link(Junction.Key, NullNode);
			NullNode->Link(TargetNode);
			
			continue;
		}
		
		// Pick our cell to use to spawn
		const int32 CellInputIndex = CellInputWeightedIndices.TwistedValue(Random);
		FNCellInputData CellInputData = Context->CellInputData[CellInputIndex];
		
		// Pick the junction of the cell we are going to use
		TArray<int32> TargetJunctionKeys = CellInputData.GetJunctionKeys(Junction.Value->SocketSize);
		const int TargetJunctionKeyIndex = Random.IntegerRange(0, TargetJunctionKeys.Num()-1);
		const int TargetJunctionKey = TargetJunctionKeys[TargetJunctionKeyIndex];
		FNCellJunctionDetails* TargetJunctionDetails = CellInputData.Junctions.Find(TargetJunctionKey);
		
		// Figure out rotations
		FQuat SourceJunctionQuat = Junction.Value->RootRelativeRotation.Quaternion();
		FQuat TargetJunctionQuat = TargetJunctionDetails->RootRelativeRotation.Quaternion();

		// The cell's world rotation = BoneRotation * Inverse(JunctionRelativeRotation)
		// This makes the junction align with the bone socket
		FQuat TargetJunctionWorldQuat = SourceJunctionQuat * TargetJunctionQuat.Inverse();
		FRotator TargetJunctionWorldRotation = TargetJunctionWorldQuat.Rotator(); 

		// Step 3: Calculate the cell's world position
		// Transform the junction's relative location by the cell's world rotation, then offset from bone position
		FVector TargetJunctionWorldOffset = TargetJunctionWorldQuat.RotateVector(Junction.Value->RootRelativeLocation);
		FVector TargetJunctionWorldPosition = Junction.Value->RootRelativeLocation - TargetJunctionWorldOffset;
	
		FNProcGenGraphCellNode* TargetCellNode = FNProcGenGraphNodeFactory::CreateCellNode(&CellInputData, TargetJunctionWorldPosition, TargetJunctionWorldRotation);
		Context->CellGraph->RegisterNode(TargetCellNode);
		
		TargetNode->Link(Junction.Key, TargetCellNode);
		TargetCellNode->Link(TargetJunctionKey, TargetNode);
	}
	
	
	// TODO: Recursive looking now?
	
	
	// GET Open Junctions // make method on node?
	
	
	// TODO: We need to create a dual structure
	// 1 holds just the raw cell/position/rotation
	// 2 holds more intrictate details about choices, junctions filled to start, and linking to OTHER cells?
	
	
	// IMPLEMENT
	// - use bones position/rotation/socket size to find a matching cell that allows for first

	// - loop through junctions on cell
	//		- decide if fill or not
	//		- find cell to attach
	
	
	// - loop throw all newly added cells and their junctions 
	//      - decide if fill or not
	//      - find cell to attach
	
		
	// REMINDERS
	// - Need a quick way to make sure that a spawned cell is INSIDE the volume
	// - at a high-level we could represent the choice structure as a linkedlist/graph of sorts
	// - when we rotate cell upon spawning we need to adjust the rotations of all the things under it that are root relative, junctions ? do we care

	Context->bSuccessful = true;
}
