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

        const TArray<FPCGPoint>& RawPoints = PointData->GetPoints();
        if (RawPoints.Num() < 2) {
            Outputs.Add(Input);
            continue;
        }

        TArray<FPCGPoint> SortedPoints;
        TArray<bool> Visited;
        Visited.Init(false, RawPoints.Num());
        int32 CurrentIndex = 0;
        
        // Do we need to find the starting position?
        if (Settings->bLeftMostStartingPoint)
        {
            float MinX = TNumericLimits<float>::Max();
            for(int32 i = 0; i < RawPoints.Num(); ++i) {
                if(RawPoints[i].Transform.GetLocation().X < MinX) {
                    MinX = RawPoints[i].Transform.GetLocation().X;
                    CurrentIndex = i;
                }
            }
        }

        // Build Sorted Points (Nearest Neighbor)
        while (CurrentIndex != INDEX_NONE) {
            Visited[CurrentIndex] = true;
            SortedPoints.Add(RawPoints[CurrentIndex]);

            int32 NextIdx = INDEX_NONE;
            float ClosestDistSq = TNumericLimits<float>::Max();

            for (int32 i = 0; i < RawPoints.Num(); ++i) {
                if (Visited[i]) continue;

                float DistSq = FVector::DistSquared(RawPoints[CurrentIndex].Transform.GetLocation(), RawPoints[i].Transform.GetLocation());
                if (DistSq < ClosestDistSq) {
                    ClosestDistSq = DistSq;
                    NextIdx = i;
                }
            }
            CurrentIndex = NextIdx;
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
            
            for (int32 i = 0; i < NumPoints; ++i) {
                
                // Direction to "next" point
                FVector Direction = FVector::ZeroVector;
                if (i < NumPointsMinusOne) {
                    Direction = (OutPoints[i+1].Transform.GetLocation() - OutPoints[i].Transform.GetLocation()).GetSafeNormal();
                } 
                else {
                    Direction = (OutPoints[i].Transform.GetLocation() - OutPoints[i-1].Transform.GetLocation()).GetSafeNormal();
                }
                
                if (i > 0 && i < NumPointsMinusOne)
                {
                    FVector PreviousDirection = DirAttr->GetValue(OutPoints[i-1].MetadataEntry);
                    
                    // Inside your loop (starting from the second point):
                    // A value near 0.0 is a very slight bend, while a value near 1.0 or -1.0 is a sharp 90-degree turn.
                    float TurnValue = FVector::DotProduct(FVector::CrossProduct(PreviousDirection, Direction), FVector::UpVector);
                    TurnAttr->SetValue(OutPoints[i].MetadataEntry, TurnValue);
                }
                
               
                
                DirAttr->SetValue(OutPoints[i].MetadataEntry, Direction);
            }
            
            if (Settings->bIsLoop)
            {
                // Handle loop back to point 0
            }
        }

        // Finalize
        Outputs.Add_GetRef(Input).Data = OutputData;
    }
    return true;
}