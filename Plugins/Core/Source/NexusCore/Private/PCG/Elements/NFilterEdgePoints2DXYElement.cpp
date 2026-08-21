// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "PCG/Elements/NFilterEdgePoints2DXYElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"
#include "Async/ParallelFor.h"
#include "Metadata/PCGMetadata.h"

namespace NEXUS::Core::FilterEdgePoints2DXY
{
	/**
	 * Neighbour count that marks a point as fully surrounded. On a regular grid this is exactly the interior
	 * condition: an interior point has eight neighbours, an edge point five, and a corner three.
	 */
	static constexpr int32 InteriorNeighborCount = 8;
}

TArray<FPCGPinProperties> UNFilterEdgePoints2DXYSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNFilterEdgePoints2DXYSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInFilterLabel, EPCGDataType::Point);
	PinProperties.Emplace(PCGPinConstants::DefaultOutFilterLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNFilterEdgePoints2DXYSettings::CreateElement() const {
	return MakeShared<FNFilterEdgePoints2DXYElement>();
}

void FNFilterEdgePoints2DXYElement::AssignEdgeDepths(const int32 NumPoints, const int32 MaxDepth, TFunctionRef<void(int32, TFunctionRef<void(int32)>)> VisitNeighbors, TArray<int32>& OutDepths)
{
	using namespace NEXUS::Core::FilterEdgePoints2DXY;

	OutDepths.Reset();
	OutDepths.SetNumZeroed(NumPoints);
	if (NumPoints == 0)
	{
		return;
	}

	// A depth below one has no sensible reading — taking no rings would turn the node into a passthrough —
	// so fall back to the single ring the node has always produced.
	const int32 RingCount = FMath::Max(MaxDepth, 1);

	// Live neighbour tallies, kept up to date as rings come off. This is what lets a later round read the same
	// counts a fresh pass over the remaining points would have produced.
	TArray<int32> NeighborCounts;
	NeighborCounts.SetNumZeroed(NumPoints);
	ParallelFor(NumPoints, [&NeighborCounts, &VisitNeighbors](const int32 Index)
	{
		int32 NeighborCount = 0;
		VisitNeighbors(Index, [&NeighborCount](int32) { ++NeighborCount; });
		NeighborCounts[Index] = NeighborCount;
	});

	// The outline itself: everything the plain edge test would have found.
	TArray<int32> Frontier;
	for (int32 Index = 0; Index < NumPoints; ++Index)
	{
		if (NeighborCounts[Index] < InteriorNeighborCount)
		{
			OutDepths[Index] = 1;
			Frontier.Add(Index);
		}
	}

	// Take one whole ring at a time. Only points touching the ring just removed can have lost a neighbour, so
	// each round walks the frontier rather than the entire set; an input thinner than the requested depth
	// simply consumes itself and the frontier runs dry early.
	for (int32 Ring = 1; Ring < RingCount && !Frontier.IsEmpty(); ++Ring)
	{
		TArray<int32> NextFrontier;
		for (const int32 Index : Frontier)
		{
			VisitNeighbors(Index, [&OutDepths, &NeighborCounts, &NextFrontier, Ring](const int32 NeighborIndex)
			{
				// Points already claimed by this or an earlier ring are gone from the working set; their tallies
				// stopped meaning anything the moment they came off, and re-testing them would double-count.
				if (OutDepths[NeighborIndex] != 0)
				{
					return;
				}

				if (--NeighborCounts[NeighborIndex] < InteriorNeighborCount)
				{
					OutDepths[NeighborIndex] = Ring + 1;
					NextFrontier.Add(NeighborIndex);
				}
			});
		}

		Frontier = MoveTemp(NextFrontier);
	}
}

bool FNFilterEdgePoints2DXYElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterEdgePoints2DXYSettings* Settings = Context->GetInputSettings<UNFilterEdgePoints2DXYSettings>();
    if (!Settings) return true;

    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

    for (const FPCGTaggedData& Input : Inputs)
    {
        const UPCGPointData* OriginalData = Cast<UPCGPointData>(Input.Data);
        if (!OriginalData || OriginalData->GetPoints().IsEmpty()) continue;

        const TArray<FPCGPoint>& Points = OriginalData->GetPoints();
        const int32 PointCount = Points.Num();
        const float SearchRadius = Settings->FilterParams.Spacing * 1.5f;
        const float SquaredSearchRadius = SearchRadius * SearchRadius;
    	const PCGPointOctree::FPointOctree& Octree = OriginalData->GetPointOctree();

    	// Neighbourhood lookup shared by the initial count and every ring that comes off after it, so the
    	// octree query and the XY-only distance test are stated once.
    	auto VisitNeighbors = [&Points, &Octree, SearchRadius, SquaredSearchRadius](const int32 Index, TFunctionRef<void(int32)> OnNeighbor)
    	{
    		const FVector Center = Points[Index].Transform.GetLocation();
    		const FBox CenterBox(Center - FVector(SearchRadius), Center + FVector(SearchRadius));

    		Octree.FindElementsWithBoundsTest(CenterBox, [&](const PCGPointOctree::FPointRef& NeighborRef)
    		{
    			if (NeighborRef.Index != Index)
    			{
    				if (FVector::DistSquaredXY(Center, Points[NeighborRef.Index].Transform.GetLocation()) <= SquaredSearchRadius)
    				{
    					OnNeighbor(NeighborRef.Index);
    				}
    			}
    		});
    	};

    	TArray<int32> EdgeDepths;
    	AssignEdgeDepths(PointCount, Settings->FilterParams.Depth, VisitNeighbors, EdgeDepths);

        UPCGPointData* InsideData = NewObject<UPCGPointData>();
        InsideData->InitializeFromData(OriginalData);
        UPCGPointData* OutsideData = NewObject<UPCGPointData>();
        OutsideData->InitializeFromData(OriginalData);

    	// Both outputs get the attribute so the two pins keep a matching schema and can be merged downstream;
    	// the interior simply carries zero, which reads as "never taken as an edge".
    	const bool bWriteEdgeDepth = Settings->FilterParams.bWriteEdgeDepth;
    	UPCGMetadata* InsideMetadata = bWriteEdgeDepth ? InsideData->MutableMetadata() : nullptr;
    	UPCGMetadata* OutsideMetadata = bWriteEdgeDepth ? OutsideData->MutableMetadata() : nullptr;
    	FPCGMetadataAttribute<int32>* InsideAttribute = InsideMetadata
			? InsideMetadata->FindOrCreateAttribute<int32>(Settings->FilterParams.EdgeDepthAttributeName, 0, /*bAllowsInterpolation=*/false, /*bOverrideParent=*/true)
			: nullptr;
    	FPCGMetadataAttribute<int32>* OutsideAttribute = OutsideMetadata
			? OutsideMetadata->FindOrCreateAttribute<int32>(Settings->FilterParams.EdgeDepthAttributeName, 0, /*bAllowsInterpolation=*/false, /*bOverrideParent=*/true)
			: nullptr;

        for (int32 i = 0; i < PointCount; ++i)
        {
        	const int32 EdgeDepth = EdgeDepths[i];
        	const bool bIsInterior = EdgeDepth == 0;
        	FPCGPoint& NewPoint = bIsInterior
				? InsideData->GetMutablePoints().Add_GetRef(Points[i])
				: OutsideData->GetMutablePoints().Add_GetRef(Points[i]);

        	if (FPCGMetadataAttribute<int32>* Attribute = bIsInterior ? InsideAttribute : OutsideAttribute)
        	{
        		// Entries arrive copied from (and potentially parented to) the input's metadata, so give each
        		// point a local entry first or the writes collapse onto whatever entry they happen to share.
        		(bIsInterior ? InsideMetadata : OutsideMetadata)->InitializeOnSet(NewPoint.MetadataEntry);
        		Attribute->SetValue(NewPoint.MetadataEntry, EdgeDepth);
        	}
        }

        FPCGTaggedData& InsideOutput = Context->OutputData.TaggedData.Emplace_GetRef();
        InsideOutput.Data = InsideData;
        InsideOutput.Pin = PCGPinConstants::DefaultInFilterLabel;

        FPCGTaggedData& OutsideOutput = Context->OutputData.TaggedData.Emplace_GetRef();
        OutsideOutput.Data = OutsideData;
        OutsideOutput.Pin = PCGPinConstants::DefaultOutFilterLabel;
    }

    return true;
}
