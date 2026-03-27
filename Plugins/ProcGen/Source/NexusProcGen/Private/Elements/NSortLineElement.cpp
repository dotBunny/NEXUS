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
        
        // Do we need to find the starting position?
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
        if (Settings->bWriteDirectionAttribute)
        {
            FPCGMetadataAttribute<FVector>* DirAttr = OutputData->Metadata->FindOrCreateAttribute<FVector>(Settings->DirectionAttributeName, FVector::ZeroVector, false, true);
            FPCGMetadataAttribute<float>* TurnAttr = OutputData->Metadata->FindOrCreateAttribute<float>(Settings->TurnAttributeName, false, false, true);
            
            FVector PreviousNextDirectionCache;
            for (int32 i = 0; i < NumPoints; ++i) {
                
                // Direction to "next" point
                FVector NextDirection = FVector::ZeroVector;
                if (i < NumPointsMinusOne)  // Not the last point
                {
                    NextDirection = (OutPoints[i+1].Transform.GetLocation() - OutPoints[i].Transform.GetLocation()).GetSafeNormal();
                } 
                else if (Settings->bIsLoop) // Is last point and loop
                {
                    NextDirection = (OutPoints[0].Transform.GetLocation() - OutPoints[i].Transform.GetLocation()).GetSafeNormal();
                }
                
                if (i > 0)
                {
                    // Inside your loop (starting from the second point):
                    // A value near 0.0 is a very slight bend, while a value near 1.0 or -1.0 is a sharp 90-degree turn.
                    float TurnValue = FVector::DotProduct(FVector::CrossProduct(PreviousNextDirectionCache, NextDirection), FVector::UpVector);
                    TurnAttr->SetValue(OutPoints[i].MetadataEntry, TurnValue);
                }
                
                DirAttr->SetValue(OutPoints[i].MetadataEntry, NextDirection);
                PreviousNextDirectionCache = NextDirection;
            }
            
            if (Settings->bIsLoop)
            {
                
                float TurnValue = FVector::DotProduct(FVector::CrossProduct(DirAttr->GetValue(OutPoints[0].MetadataEntry), DirAttr->GetValue(OutPoints[NumPointsMinusOne].MetadataEntry)), FVector::UpVector);
                TurnAttr->SetValue(OutPoints[0].MetadataEntry, TurnValue);
                // Handle loop back to point 0
            }
        }

        // Finalize
        Outputs.Add_GetRef(Input).Data = OutputData;
    }
    return true;
}