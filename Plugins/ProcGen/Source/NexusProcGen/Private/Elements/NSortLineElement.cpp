// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NSortLineElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"


TArray<FPCGPinProperties> UNSortLineSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
    return PinProperties;
}

TArray<FPCGPinProperties> UNSortLineSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
    return PinProperties;
}

FPCGElementPtr UNSortLineSettings::CreateElement() const {
    return MakeShared<FNSortLineElement>();
}

// TODO:
//  - segmentindex
//  - subsegmentindex
//  - segmentlength
// - Detect winding order | or use fixed
// - rotate to facing direction

UE_DISABLE_OPTIMIZATION
bool FNSortLineElement::ExecuteInternal(FPCGContext* Context) const {
    const UNSortLineSettings* Settings = Context->GetInputSettings<UNSortLineSettings>();
    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
    TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

    for (const FPCGTaggedData& Input : Inputs) 
    {
        const UPCGPointData* PointData = Cast<UPCGPointData>(Input.Data);
        if (!PointData) continue;

        const TArray<FPCGPoint>& UnsortedPoints = PointData->GetPoints();
        if (UnsortedPoints.Num() < 2) {
            Outputs.Add(Input);
            continue;
        }

        TArray<FPCGPoint> SortedPoints;
        TArray<bool> Visited;
        Visited.Init(false, UnsortedPoints.Num());
        int32 CurrentIndex = 0;
        
        // Find starting point for consistency
        if (Settings->bLeftMostStartingPoint)
        {
            float MinimumPositionX = TNumericLimits<float>::Max();
            for(int32 i = 0; i < UnsortedPoints.Num(); ++i) {
                if(UnsortedPoints[i].Transform.GetLocation().X < MinimumPositionX) {
                    MinimumPositionX = UnsortedPoints[i].Transform.GetLocation().X;
                    CurrentIndex = i;
                }
            }
        }

        // Build Sorted Points (Nearest Neighbor)
        while (CurrentIndex != INDEX_NONE) 
        {
            Visited[CurrentIndex] = true;
            SortedPoints.Add(UnsortedPoints[CurrentIndex]);

            int32 NextIndex = INDEX_NONE;
            float ClosestDistanceSquared = TNumericLimits<float>::Max();

            for (int32 i = 0; i < UnsortedPoints.Num(); ++i) 
            {
                if (Visited[i])
                {
                    continue;
                }

                const float DistanceSquared = FVector::DistSquared(UnsortedPoints[CurrentIndex].Transform.GetLocation(), UnsortedPoints[i].Transform.GetLocation());
                if (DistanceSquared < ClosestDistanceSquared) 
                {
                    ClosestDistanceSquared = DistanceSquared;
                    NextIndex = i;
                }
            }
            CurrentIndex = NextIndex;
        }

        // Build Output Data
        UPCGPointData* OutputData = NewObject<UPCGPointData>();
        OutputData->InitializeFromData(PointData);
        TArray<FPCGPoint>& OutPoints = OutputData->GetMutablePoints();
        OutPoints = SortedPoints;
        
        const int NumPoints = OutPoints.Num();
        const int NumPointsMinusOne = NumPoints - 1;
        
        // Should we figure out the next direction?
        if (Settings->bWriteMetadata)
        {
            int32 SegmentIndex = 0;
        	int32 SegmentLength = 1;
        	int32 SubsegmentIndex = 0;
            TArray<int32> CachedSegmentLengths;
        	
            FPCGMetadataAttribute<FVector>* DirAttr = OutputData->Metadata->FindOrCreateAttribute<FVector>(Settings->NextGridDirectionAttributeName, FVector::ZeroVector, false, true);
            FPCGMetadataAttribute<float>* TurnAttr = OutputData->Metadata->FindOrCreateAttribute<float>(Settings->TurnAttributeName, false, false, true);
            FPCGMetadataAttribute<int32>* SegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentIndexAttributeName, false, false, true);
            FPCGMetadataAttribute<int32>* SubsegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SubsegmentIndexAttributeName, false, false, true);
          
            FVector PreviousNextGridDirection;
            for (int32 i = 0; i < NumPoints; ++i) {
                
                // Ensure metadata existence
                if (OutPoints[i].MetadataEntry == -1)
                {
                    OutputData->Metadata->InitializeOnSet(OutPoints[i].MetadataEntry);
                }
                
                // Direction to "next" point
                FVector NextGridDirection = FVector::ZeroVector;
                if (i < NumPointsMinusOne)  // Not the last point
                {
                    NextGridDirection = (OutPoints[i+1].Transform.GetLocation() - OutPoints[i].Transform.GetLocation()).GetSafeNormal();
                } 
                else if (Settings->bIsLoop) // Is last point and loop
                {
                    NextGridDirection = (OutPoints[0].Transform.GetLocation() - OutPoints[i].Transform.GetLocation()).GetSafeNormal();
                }
            	
            	
                
                if (i > 0)
                {
                    // 1/-1 90 degrees
                    // inbetween/close to 0 is slight
                    // 0 straight
                    float TurnValue = FVector::DotProduct(FVector::CrossProduct(PreviousNextGridDirection, NextGridDirection), FVector::UpVector);
                    TurnAttr->SetValue(OutPoints[i].MetadataEntry, TurnValue);
                	
                	
                	// // We will need to clean up the loop
                	// if (TurnValue != 0)
                	// {
                	// 	CachedSegmentLengths.Add(SegmentLength);
	                //
                	// 	SegmentIndex++;
                	// 	SubsegmentIndex = 0;
                	// 	SegmentLength = 1;
                	// 	
                	// 	SegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, SegmentIndex);
                	// 	SubsegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, SubsegmentIndex);
                	// 	
                	// 	SegmentIndex++;
                	// }
	                // else
	                // {
	                // 	SubsegmentIndex++;
	                // 	SegmentLength++;
	                // }
                }
                else
                {
                	// SubsegmentIndex++;
                	// SegmentLength++;
                }
            	
            	
            	
            
            	
            	
                DirAttr->SetValue(OutPoints[i].MetadataEntry, NextGridDirection);
                PreviousNextGridDirection = NextGridDirection;
            	
            	
            	
            }
            
            // Handle loop back to point 0
            if (Settings->bIsLoop)
            {
                float TurnValue = FVector::DotProduct(FVector::CrossProduct(DirAttr->GetValue(OutPoints[NumPointsMinusOne].MetadataEntry), DirAttr->GetValue(OutPoints[0].MetadataEntry)), FVector::UpVector);
                TurnAttr->SetValue(OutPoints[0].MetadataEntry, TurnValue);
            }
        	
        	// Write out lengths
        	if (Settings->bWriteSegmentLength)
        	{
    //     		FPCGMetadataAttribute<int32>* SegmentLengthAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentLengthAttributeName, false, false, true);
				// for (int32 i = 0; i < NumPoints; ++i)
				// {
				// 	int TempSegIndex = SegmentIndexAttr->GetValue(OutPoints[i].MetadataEntry);
				// 	int TempValue = CachedSegmentLengths[TempSegIndex];
				// 	SegmentLengthAttr->SetValue(OutPoints[i].MetadataEntry, TempValue);
				// }
        	}
        }

        // Finalize
        Outputs.Add_GetRef(Input).Data = OutputData;
    }
    return true;
}
UE_ENABLE_OPTIMIZATION