// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NFilterEdgePoints2DElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"

TArray<FPCGPinProperties> UNFilterEdgePoints2DSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNFilterEdgePoints2DSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInFilterLabel, EPCGDataType::Point);
	PinProperties.Emplace(PCGPinConstants::DefaultOutFilterLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNFilterEdgePoints2DSettings::CreateElement() const {
	return MakeShared<FNFilterEdgePoints2DElement>();
}

bool FNFilterEdgePoints2DElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterEdgePoints2DSettings* Settings = Context->GetInputSettings<UNFilterEdgePoints2DSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGPointData* OriginalData = Cast<UPCGPointData>(Input.Data);
		if (!OriginalData) continue;

		const TArray<FPCGPoint>& Points = OriginalData->GetPoints();
		const PCGPointOctree::FPointOctree& Octree = OriginalData->GetPointOctree();

		UPCGPointData* InsideData = NewObject<UPCGPointData>();
		InsideData->InitializeFromData(OriginalData);
		UPCGPointData* OutsideData = NewObject<UPCGPointData>();
		OutsideData->InitializeFromData(OriginalData);

		const float SearchRadius = Settings->FilterParams.Spacing * 1.5f; // We want to build a bit of buffer in to the spatial query
		const float SquaredSearchRadius = SearchRadius * SearchRadius;

		for (int32 i = 0; i < Points.Num(); ++i)
		{
			const FPCGPoint& CurrentPoint = Points[i];
			FVector Center = CurrentPoint.Transform.GetLocation();
			int32 NeighborCount = 0;
			FBox CenterBox(Center - FVector(SearchRadius), Center + FVector(SearchRadius));

			Octree.FindElementsWithBoundsTest(CenterBox, [&](const PCGPointOctree::FPointRef& NeighborRef)
			{
				if (NeighborRef.Index != i)
				{
					const FPCGPoint& NeighborPoint = Points[NeighborRef.Index];
					const float DistanceSquared = FVector::DistSquaredXY(Center, NeighborPoint.Transform.GetLocation());
                    
					if (DistanceSquared <= SquaredSearchRadius)
					{
						NeighborCount++;
					}
				}
			});

			if (NeighborCount == 8)
			{
				InsideData->GetMutablePoints().Add(CurrentPoint);
			}
			else
			{
				OutsideData->GetMutablePoints().Add(CurrentPoint);
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