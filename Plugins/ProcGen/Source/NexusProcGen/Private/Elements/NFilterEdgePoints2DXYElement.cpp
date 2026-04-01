// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NFilterEdgePoints2DXYElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"
#include "Async/ParallelFor.h"

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

bool FNFilterEdgePoints2DXYElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterEdgePoints2DXYSettings* Settings = Context->GetInputSettings<UNFilterEdgePoints2DXYSettings>();
    if (!Settings) return true;

    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

    for (const FPCGTaggedData& Input : Inputs)
    {
        const UPCGPointData* OriginalData = Cast<UPCGPointData>(Input.Data);
        if (!OriginalData || OriginalData->GetPoints().Num() == 0) continue;

        const TArray<FPCGPoint>& Points = OriginalData->GetPoints();
        const int32 PointCount = Points.Num();
        TArray<int8> Classification;
        Classification.Init(0, PointCount);
        const float SearchRadius = Settings->FilterParams.Spacing * 1.5f;
        const float SquaredSearchRadius = SearchRadius * SearchRadius;
    	const PCGPointOctree::FPointOctree& Octree = OriginalData->GetPointOctree();

        ParallelFor(PointCount, [&](const int32 Index)
        {
            const FPCGPoint& CurrentPoint = Points[Index];
            const FVector Center = CurrentPoint.Transform.GetLocation();
            int32 NeighborCount = 0;
            const FBox CenterBox(Center - FVector(SearchRadius), Center + FVector(SearchRadius));

            Octree.FindElementsWithBoundsTest(CenterBox, [&](const PCGPointOctree::FPointRef& NeighborRef)
            {
                if (NeighborRef.Index != Index)
                {
                    if (FVector::DistSquaredXY(Center, Points[NeighborRef.Index].Transform.GetLocation()) <= SquaredSearchRadius)
                    {
                        NeighborCount++;
                    }
                }
            });

            if (NeighborCount >= 8)
            {
                Classification[Index] = 1;
            }
        });

        UPCGPointData* InsideData = NewObject<UPCGPointData>();
        InsideData->InitializeFromData(OriginalData);
        UPCGPointData* OutsideData = NewObject<UPCGPointData>();
        OutsideData->InitializeFromData(OriginalData);

        for (int32 i = 0; i < PointCount; ++i)
        {
            if (Classification[i] == 1)
            {
                InsideData->GetMutablePoints().Add(Points[i]);
            }
            else
            {
                OutsideData->GetMutablePoints().Add(Points[i]);
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