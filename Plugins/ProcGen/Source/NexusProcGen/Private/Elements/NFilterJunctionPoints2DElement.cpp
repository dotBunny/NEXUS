// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NFilterJunctionPoints2DElement.h"
#include "Data/PCGPointData.h"
#include "PCGContext.h"
#include "Async/ParallelFor.h"

TArray<FPCGPinProperties> UNFilterJunctionPoints2DSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(SourceAttribute, EPCGDataType::Point);
    PinProperties.Emplace(JunctionsAttribute, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNFilterJunctionPoints2DSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInFilterLabel, EPCGDataType::Point);
	PinProperties.Emplace(PCGPinConstants::DefaultOutFilterLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNFilterJunctionPoints2DSettings::CreateElement() const {
	return MakeShared<FNFilterJunctionPoints2DElement>();
}

bool FNFilterJunctionPoints2DElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNFilterJunctionPoints2DSettings* Settings = Context->GetInputSettings<UNFilterJunctionPoints2DSettings>();
    if (!Settings) return true;

    TArray<FPCGTaggedData> SourceInput = Context->InputData.GetInputsByPin(Settings->SourceAttribute);
    TArray<FPCGTaggedData> TargetInput = Context->InputData.GetInputsByPin(Settings->JunctionsAttribute);

	
	
	// Border points offset -> SOURCE
	// Junction offset -> TARGET
	
	// Get distances from source to nearest target (box bounds to box bounds) max distance 1.f (no need for more)
	// Filter by SocketSize
	
    return true;
}