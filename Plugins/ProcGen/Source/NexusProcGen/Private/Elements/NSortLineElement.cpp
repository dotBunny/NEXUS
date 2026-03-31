// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NSortLineElement.h"

#include "NProcGenMinimal.h"
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
        float TurnDirectionTotal = 0;
    	
        // Should we figure out the next direction?
        if (Settings->bWriteDirectionMetadata)
        {
        	TArray<float> CachedTurnValues { 0.f };
        	CachedTurnValues.Reserve(NumPoints);
        	
            FPCGMetadataAttribute<FVector>* DirAttr = OutputData->Metadata->FindOrCreateAttribute<FVector>(Settings->NextGridDirectionAttributeName, FVector::ZeroVector, false, true);
            FPCGMetadataAttribute<float>* TurnDirectionAttr = OutputData->Metadata->FindOrCreateAttribute<float>(Settings->TurnDirectionAttributeName, 0.f, false, true);
            
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
            	
                // Turn detection
                if (i > 0)
                {
                    // 1/-1 90 degrees
                    // inbetween/close to 0 is slight
                    // 0 straight
                    float TurnValue = FVector::DotProduct(FVector::CrossProduct(PreviousNextGridDirection, NextGridDirection), FVector::UpVector);
                    TurnDirectionAttr->SetValue(OutPoints[i].MetadataEntry, TurnValue);
                	CachedTurnValues.Add(TurnValue);
                	TurnDirectionTotal += TurnValue;
                }
                
            	
                DirAttr->SetValue(OutPoints[i].MetadataEntry, NextGridDirection);
                PreviousNextGridDirection = NextGridDirection;
            }
            
            // Handle loop back to point 0
            if (Settings->bIsLoop)
            {
                float TurnDirectionValue = FVector::DotProduct(FVector::CrossProduct(DirAttr->GetValue(OutPoints[NumPointsMinusOne].MetadataEntry), DirAttr->GetValue(OutPoints[0].MetadataEntry)), FVector::UpVector);
                TurnDirectionAttr->SetValue(OutPoints[0].MetadataEntry, TurnDirectionValue);
            	CachedTurnValues[0] = TurnDirectionValue;
            	TurnDirectionTotal += TurnDirectionValue;
            	
            	UE_LOG(LogNexusProcGen, Warning, TEXT("DEG: %f"), TurnDirectionTotal)
            }
        	
 
        	
        	// Figure out segment information
        	int32 SegmentIndex = 0;
        	int32 SegmentLength = 0;
        	int32 SubsegmentIndex = 0;
        	TArray<int32> CachedSegmentLengths;
        	TArray<int32> CachedSegmentIndex;
        	CachedSegmentIndex.Reserve(NumPoints);
        	
        	FPCGMetadataAttribute<int32>* SegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentIndexAttributeName, 0, false, true);
        	FPCGMetadataAttribute<int32>* SubsegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SubsegmentIndexAttributeName, 0, false, true);
        	for (int32 i = 0; i < NumPoints; ++i)
			{
				
				
				// We are in a turn
				if (CachedTurnValues[i] != 0.f)
				{
					if (i != 0)
					{
						SegmentIndex++;
						CachedSegmentLengths.Add(SegmentLength);
					}
					SegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, SegmentIndex);
					CachedSegmentIndex.Add(SegmentIndex);
					SegmentIndex++;
					SubsegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, 0);
					SubsegmentIndex = -1;
					CachedSegmentLengths.Add(1);
					SegmentLength = 0;
					
				}
				else
				{
					SegmentLength++;
					SegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, SegmentIndex);
					CachedSegmentIndex.Add(SegmentIndex);
					SubsegmentIndex++;
					SubsegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, SubsegmentIndex);
				}
			}
        	CachedSegmentLengths.Add(SegmentLength);
        	
        	// Pass to write out finalized data
        	FPCGMetadataAttribute<int32>* SegmentLengthAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentLengthAttributeName, 0, false, true);
        	for (int32 i = 0; i < NumPoints; ++i)
        	{
        		SegmentLengthAttr->SetValue(OutPoints[i].MetadataEntry, CachedSegmentLengths[CachedSegmentIndex[i]]);
        	}
        }

        // Finalize
        Outputs.Add_GetRef(Input).Data = OutputData;
    }
    return true;
}
UE_ENABLE_OPTIMIZATION