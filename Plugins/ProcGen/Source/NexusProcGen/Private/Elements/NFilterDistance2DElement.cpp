// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NFilterDistance2DElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"
#include "Async/ParallelFor.h"

TArray<FPCGPinProperties> UNFilterDistance2DSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(SourceAttribute, EPCGDataType::Point);
    PinProperties.Emplace(TargetAttribute, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNFilterDistance2DSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInFilterLabel, EPCGDataType::Point);
	PinProperties.Emplace(PCGPinConstants::DefaultOutFilterLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNFilterDistance2DSettings::CreateElement() const {
	return MakeShared<FNFilterDistance2DElement>();
}

bool FNFilterDistance2DElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterDistance2DSettings* Settings = Context->GetInputSettings<UNFilterDistance2DSettings>();
    if (!Settings) return true;

    TArray<FPCGTaggedData> SourceInput = Context->InputData.GetInputsByPin(Settings->SourceAttribute);
    TArray<FPCGTaggedData> TargetInput = Context->InputData.GetInputsByPin(Settings->TargetAttribute);
	
	
    return true;
}