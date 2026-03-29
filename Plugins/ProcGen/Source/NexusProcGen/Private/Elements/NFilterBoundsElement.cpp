// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NFilterBoundsElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"
#include "Async/ParallelFor.h"

TArray<FPCGPinProperties> UNFilterBoundsSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(SourceAttribute, EPCGDataType::Point);
    PinProperties.Emplace(TargetAttribute, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNFilterBoundsSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInFilterLabel, EPCGDataType::Point);
	PinProperties.Emplace(PCGPinConstants::DefaultOutFilterLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNFilterBoundsSettings::CreateElement() const {
	return MakeShared<FNFilterBoundsElement>();
}

bool FNFilterBoundsElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterBoundsSettings* Settings = Context->GetInputSettings<UNFilterBoundsSettings>();
    if (!Settings) return true;

    TArray<FPCGTaggedData> SourceInput = Context->InputData.GetInputsByPin(Settings->SourceAttribute);
    TArray<FPCGTaggedData> TargetInput = Context->InputData.GetInputsByPin(Settings->TargetAttribute);

	
    return true;
}