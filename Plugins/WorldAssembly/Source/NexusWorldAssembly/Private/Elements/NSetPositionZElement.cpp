// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NSetPositionZElement.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"

TArray<FPCGPinProperties> UNSetPositionZSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNSetPositionZSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNSetPositionZSettings::CreateElement() const
{
	return MakeShared<FNSetPositionZElement>();
}

bool FNSetPositionZElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNSetPositionZSettings* Settings = Context->GetInputSettings<UNSetPositionZSettings>();
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGPointData* OriginalData = Cast<UPCGPointData>(Input.Data);
		if (!OriginalData && OriginalData->GetNumPoints() == 0) continue;
		
		const TArray<FPCGPoint>& Points = OriginalData->GetPoints();
		
		UPCGPointData* OutputData = NewObject<UPCGPointData>();
		OutputData->InitializeFromData(OriginalData);
		TArray<FPCGPoint>& MutablePoints = OutputData->GetMutablePoints();
		
		for (const FPCGPoint& Point : Points)
		{
			FPCGPoint NewPoint(Point);
			
			FVector Pos = NewPoint.Transform.GetLocation();
			Pos.Z = Settings->SetParams.ZValue;
			NewPoint.Transform.SetLocation(Pos);
			
			MutablePoints.Add(NewPoint);
		}
		
		FPCGTaggedData& TaggedData = Context->OutputData.TaggedData.Emplace_GetRef();
		TaggedData.Pin = PCGPinConstants::DefaultOutputLabel;
		TaggedData.Data = OutputData;
	}
	return true;
}
