// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"
#include "Generation/NCellOutputData.h"
#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Context(ContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{
}

// TODO: Spin up multiple of these tasks? with different sub-seeds? Maybe we make a seperate task under this

void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
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
	int StartCellJunctionKeyIndex = Random.IntegerRange(0, StartCellJunctionKeys.Num()-1);
	FNCellJunctionDetails* StartCellJunctionDetails = StartCellInputData.Junctions.Find(StartCellJunctionKeyIndex);
	
	
	// START figure out rotation
	
	
	// Establish a base understanding of the rotations/directions
	FVector JunctionForward = StartCellJunctionDetails->RootRelativeRotation.Vector();
	FVector BoneForward = BoneData.WorldRotation.Vector();
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
	
	// END figure out rotaion
	
	FNCellOutputData StartCellOutputData;
	StartCellOutputData.WorldPosition = CellWorldPosition;
	StartCellOutputData.WorldRotation = CellWorldRotation;
	StartCellOutputData.Template = StartCellInputData.Template;
	Context->CellOutputData.Add(StartCellOutputData);
	
	
	
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
