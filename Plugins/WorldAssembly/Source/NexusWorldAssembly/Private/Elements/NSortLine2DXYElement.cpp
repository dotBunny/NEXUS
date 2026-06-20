// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NSortLine2DXYElement.h"

#include "Data/PCGPointData.h"
#include "PCGContext.h"


TArray<FPCGPinProperties> UNSortLine2DXYSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
    return PinProperties;
}

TArray<FPCGPinProperties> UNSortLine2DXYSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
    return PinProperties;
}

FPCGElementPtr UNSortLine2DXYSettings::CreateElement() const {
    return MakeShared<FNSortLine2DXYElement>();
}

void FNSortLine2DXYElement::ClassifyLine(const TArray<float>& TurnValues, const float TurnTolerance, TArray<FNSortLinePointInfo>& OutInfo)
{
    const int32 NumPoints = TurnValues.Num();
    OutInfo.Reset();
    OutInfo.SetNum(NumPoints);
    if (NumPoints == 0) return;

    int32 SegmentIndex = 0;
    int32 SegmentLength = 0;
    int32 SubsegmentIndex = 0;
    TArray<int32> SegmentLengths;     // Length of each segment, addressed by segment index.
    TArray<int32> PointSegmentIndex;  // Segment index owning each point.
    PointSegmentIndex.Reserve(NumPoints);

    for (int32 i = 0; i < NumPoints; ++i)
    {
        const float TurnValue = TurnValues[i];

        // A near-zero turn (within the deadzone) is a straight wall; anything past it is a corner. Classify the
        // corner by the sign of the turn so off-grid input still resolves to a Left/Right name rather than
        // silently keeping the default classification, while exact 0 / +/-1 grid input is unaffected (WA-3).
        if (FMath::Abs(TurnValue) > TurnTolerance)
        {
            OutInfo[i].Part = TurnValue > 0.f ? ENSortLinePart::Left90 : ENSortLinePart::Right90;

            // Close the wall run leading into this corner (point 0 has no preceding run to close).
            if (i != 0)
            {
                SegmentIndex++;
                SegmentLengths.Add(SegmentLength);
            }
            OutInfo[i].SegmentIndex = SegmentIndex;
            PointSegmentIndex.Add(SegmentIndex);
            SegmentIndex++;

            OutInfo[i].SubsegmentIndex = 0;
            SubsegmentIndex = 0;
            SegmentLengths.Add(1);  // A corner is its own length-1 segment.
            SegmentLength = 0;
        }
        else
        {
            OutInfo[i].Part = ENSortLinePart::Wall;
            SegmentLength++;
            OutInfo[i].SegmentIndex = SegmentIndex;
            PointSegmentIndex.Add(SegmentIndex);
            OutInfo[i].SubsegmentIndex = SubsegmentIndex++;  // 0-based position within the wall run.
        }
    }
    SegmentLengths.Add(SegmentLength);  // Close the trailing wall run.

    // Resolve each point's owning-segment length now that every segment is closed.
    for (int32 i = 0; i < NumPoints; ++i)
    {
        OutInfo[i].SegmentLength = SegmentLengths[PointSegmentIndex[i]];
    }
}

bool FNSortLine2DXYElement::ExecuteInternal(FPCGContext* Context) const {
    const UNSortLine2DXYSettings* Settings = Context->GetInputSettings<UNSortLine2DXYSettings>();
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
    	// It seems that the grid fill always ends up in a clockwise-wind.
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

        const int32 NumPoints = OutPoints.Num();
        const int32 NumPointsMinusOne = NumPoints - 1;


        // Should we figure out the next direction?
        if (Settings->bBuildAdditionalMetadata)
        {
        	float TurnDirectionTotal = 0;
        	TArray CachedTurnValues { 0.f };
        	CachedTurnValues.Reserve(NumPoints);

            FPCGMetadataAttribute<FVector2D>* NextGridDirectionAttr = OutputData->Metadata->FindOrCreateAttribute<FVector2D>(Settings->NextGridDirectionAttributeName, FVector2D::ZeroVector, false, true);
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
                    // in-between is slight
                    // 0 straight
                    float TurnValue = FVector::DotProduct(FVector::CrossProduct(PreviousNextGridDirection, NextGridDirection), FVector::UpVector);
                    TurnDirectionAttr->SetValue(OutPoints[i].MetadataEntry, TurnValue);
                	CachedTurnValues.Add(TurnValue);
                	TurnDirectionTotal += TurnValue;
                }

                NextGridDirectionAttr->SetValue(OutPoints[i].MetadataEntry, NextGridDirection);
                PreviousNextGridDirection = NextGridDirection;
            }

            // Handle loop back to point 0
            if (Settings->bIsLoop)
            {
            	FVector2D LHS = NextGridDirectionAttr->GetValue(OutPoints[NumPointsMinusOne].MetadataEntry);
            	FVector2D RHS = NextGridDirectionAttr->GetValue(OutPoints[0].MetadataEntry);
                float TurnDirectionValue = FVector::DotProduct(FVector::CrossProduct(FVector(LHS.X, LHS.Y, 0), FVector(RHS.X, RHS.Y, 0)), FVector::UpVector);
                TurnDirectionAttr->SetValue(OutPoints[0].MetadataEntry, TurnDirectionValue);
            	CachedTurnValues[0] = TurnDirectionValue;
            	TurnDirectionTotal += TurnDirectionValue;
            }

        	// We're going to use this to figure out what way to rotate to face walls
        	bool IsClockwise = TurnDirectionTotal > 0.f;

        	// Classify every point and resolve its segment bookkeeping (see FNSortLine2DXYElement::ClassifyLine).
        	TArray<FNSortLinePointInfo> PointInfos;
        	ClassifyLine(CachedTurnValues, NEXUS::WorldAssembly::SortLine::TurnDeadzone, PointInfos);

        	FPCGMetadataAttribute<int32>* SegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentIndexAttributeName, 0, false, true);
        	FPCGMetadataAttribute<int32>* SubsegmentIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SubsegmentIndexAttributeName, 0, false, true);
        	FPCGMetadataAttribute<FName>* PartNameAttr = OutputData->Metadata->FindOrCreateAttribute<FName>(Settings->PointNameAttributeName, Settings->DefaultPointName, false, true);

        	for (int32 i = 0; i < NumPoints; ++i)
        	{
        		const FNSortLinePointInfo& Info = PointInfos[i];
        		switch (Info.Part)
        		{
        			case ENSortLinePart::Left90:
        				PartNameAttr->SetValue(OutPoints[i].MetadataEntry, Settings->Left90PointName);
        				break;
        			case ENSortLinePart::Right90:
        				PartNameAttr->SetValue(OutPoints[i].MetadataEntry, Settings->Right90PointName);
        				break;
        			default:
        				PartNameAttr->SetValue(OutPoints[i].MetadataEntry, Settings->WallPointName);
        				break;
        		}
        		SegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, Info.SegmentIndex);
        		SubsegmentIndexAttr->SetValue(OutPoints[i].MetadataEntry, Info.SubsegmentIndex);
        	}

        	// Pass to write out finalized data
        	FPCGMetadataAttribute<int32>* SegmentLengthAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->SegmentLengthAttributeName, 0, false, true);
        	FPCGMetadataAttribute<float>* FacingRotationAttr = OutputData->Metadata->FindOrCreateAttribute<float>(Settings->FacingRotationAttributeName, 0, false, true);
        	FPCGMetadataAttribute<FName>* FacingCardinalAttr = OutputData->Metadata->FindOrCreateAttribute<FName>(Settings->FacingCardinalAttributeName, Settings->DefaultCardinalName, false, true);
        	FPCGMetadataAttribute<int32>* FacingCardinalIndexAttr = OutputData->Metadata->FindOrCreateAttribute<int32>(Settings->FacingCardinalIndexAttributeName, 0, false, true);


        	FRotator FaceRotation = FRotator(0, 90, 0);
        	if (!IsClockwise)
        	{
        		FaceRotation = FaceRotation * -1.f;
        	}

        	for (int32 i = 0; i < NumPoints; ++i)
        	{
        		SegmentLengthAttr->SetValue(OutPoints[i].MetadataEntry, PointInfos[i].SegmentLength);

        		// Corners will get rotated to "Next Direction" where walls get turned to next direction + face rotation based on winding
        		FVector2D Position2D = NextGridDirectionAttr->GetValue(OutPoints[i].MetadataEntry);
        		FVector Position = FVector(Position2D.X, Position2D.Y, 0);
        		FRotator Rotation = Position.Rotation();
        		if (PointInfos[i].Part == ENSortLinePart::Wall)
        		{
        			Rotation = Rotation + FaceRotation;
        		}
        		FacingRotationAttr->SetValue(OutPoints[i].MetadataEntry, Rotation.Yaw);
        		const int32 DirectionIndex = FMath::FloorToInt((Rotation.Yaw + 45.0f) / 90.0f) & 3;
        		FacingCardinalIndexAttr->SetValue(OutPoints[i].MetadataEntry, DirectionIndex);
		        switch (DirectionIndex)
        		{
        			case 0:
        				FacingCardinalAttr->SetValue(OutPoints[i].MetadataEntry, Settings->NorthCardinalName);
        				break;
        			case 1:
        				FacingCardinalAttr->SetValue(OutPoints[i].MetadataEntry, Settings->EastCardinalName);
        				break;
        			case 2:
	        			FacingCardinalAttr->SetValue(OutPoints[i].MetadataEntry, Settings->SouthCardinalName);
        				break;
        			case 3:
        				FacingCardinalAttr->SetValue(OutPoints[i].MetadataEntry, Settings->WestCardinalName);
        				break;
        			default:
        				FacingCardinalAttr->SetValue(OutPoints[i].MetadataEntry, Settings->DefaultCardinalName);
        				break;
        		}

        		if (Settings->bRotatePointToFaceDirection)
        		{
        			OutPoints[i].Transform.SetRotation(Rotation.Quaternion());
        		}
        	}
        }

        // Finalize
        Outputs.Add_GetRef(Input).Data = OutputData;
    }
    return true;
}