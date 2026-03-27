#include "PCG/NSetPointDirectionPCGElement.h"
#include "PCGPin.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"

TArray<FPCGPinProperties> UNSetPointDirectionPCGSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
    return PinProperties;
}

TArray<FPCGPinProperties> UNSetPointDirectionPCGSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
    return PinProperties;
}

FPCGElementPtr UNSetPointDirectionPCGSettings::CreateElement() const
{
    return MakeShared<FNSetPointDirectionPCGElement>();
}

UE_DISABLE_OPTIMIZATION
bool FNSetPointDirectionPCGElement::ExecuteInternal(FPCGContext* Context) const
{
   const UNSetPointDirectionPCGSettings* Settings = Context->GetInputSettings<UNSetPointDirectionPCGSettings>();
    check(Settings);

    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
    TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

    for (const FPCGTaggedData& Input : Inputs)
    {
        const UPCGPointData* InputPointData = Cast<UPCGPointData>(Input.Data);
        
        if (!InputPointData || InputPointData->GetPoints().Num() < 2) 
        {
            Outputs.Add(Input);
            continue;
        }

        // 1. Create the new data object
        UPCGPointData* OutputPointData = NewObject<UPCGPointData>();
        
        // 2. Initialize metadata and settings from input
        OutputPointData->InitializeFromData(InputPointData);
        
        // 3. CRITICAL: Explicitly copy the points from the input to the output
        const TArray<FPCGPoint>& InPoints = InputPointData->GetPoints();
        OutputPointData->GetMutablePoints() = InPoints; // This populates the array

        // Now we can safely get a reference to the points we just copied
        TArray<FPCGPoint>& OutPoints = OutputPointData->GetMutablePoints();
        const int32 PointCount = OutPoints.Num();

        // 4. Setup the attribute
        UPCGMetadata* Metadata = OutputPointData->Metadata;
        FPCGMetadataAttribute<FRotator>* RotAttr = Metadata->FindOrCreateAttribute<FRotator>(Settings->OutputAttributeName, FRotator::ZeroRotator, true);

        for (int32 i = 0; i < PointCount; ++i)
        {
            int32 NextIndex = (i + 1) % PointCount;

            // Use OutPoints for both to ensure we are looking at the same coordinate space
            FVector CurrentPos = OutPoints[i].Transform.GetLocation();
            FVector NextPos = OutPoints[NextIndex].Transform.GetLocation();

            FVector Direction = (NextPos - CurrentPos);
            FRotator LookAtRot = Direction.IsNearlyZero() ? OutPoints[i].Transform.Rotator() : Direction.Rotation();

            // 5. Initialize the entry and set the value
            // We use InitializeOnSet to make sure the point has a unique entry in the metadata table
            Metadata->InitializeOnSet(OutPoints[i].MetadataEntry);
            RotAttr->SetValue(OutPoints[i].MetadataEntry, LookAtRot);
        }

        FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
        Output.Data = OutputPointData;
    }

    return true;
}
UE_ENABLE_OPTIMIZATION