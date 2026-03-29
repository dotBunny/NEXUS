// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NGetJunctionDataElement.h"

#include "NProcGenPCGMacros.h"
#include "NProcGenSettings.h"
#include "PCGContext.h"
#include "PCGParamData.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellJunctionDetails.h"
#include "Data/PCGPointData.h"

TArray<FPCGPinProperties> UNGetJunctionDataSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Param);
	return PinProperties;
}

TArray<FPCGPinProperties> UNGetJunctionDataSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(JunctionsAttribute, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UNGetJunctionDataSettings::CreateElement() const
{
	return MakeShared<FNGetJunctionDataElement>();
}

bool FNGetJunctionDataElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNGetJunctionDataSettings* Settings = Context->GetInputSettings<UNGetJunctionDataSettings>();
	N_PCG_JUNCTION_PREFIX
	
	const FName ComponentReferenceName = TEXT("ComponentReference");
	
	// Could have multiple pin inputs
	for (const FPCGTaggedData& Input : Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel))
	{
		const UPCGParamData* ParamData = Cast<UPCGParamData>(Input.Data);
		if (ParamData && ParamData->Metadata)
		{
			for (int64 i = 0; i < ParamData->Metadata->GetItemCountForChild(); ++i)
			{
				const FPCGMetadataAttribute<FSoftObjectPath>* ComponentAttr = 
					ParamData->Metadata->GetConstTypedAttribute<FSoftObjectPath>(ComponentReferenceName);
				
				FSoftObjectPath ComponentSoftObjectPath = ComponentAttr->GetValueFromItemKey(i);
				if (ComponentSoftObjectPath.IsValid())
				{
					UNCellJunctionComponent* JunctionComponent = Cast<UNCellJunctionComponent>(ComponentSoftObjectPath.ResolveObject());
					if (JunctionComponent != nullptr)
					{
						N_PCG_JUNCTION_DATA
					}
				}
			}
		}
	}
	
	FPCGTaggedData& TaggedData = Outputs.Emplace_GetRef();
	TaggedData.Data = OutputData;
	TaggedData.Pin = Settings->JunctionsAttribute; 
	
	return true;
}
