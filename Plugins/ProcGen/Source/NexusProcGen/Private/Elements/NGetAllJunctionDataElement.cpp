// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NGetAllJunctionDataElement.h"

#include "NProcGenPCGMacros.h"
#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "PCGComponent.h"
#include "PCGContext.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellJunctionDetails.h"
#include "Data/PCGPointData.h"
#include "Subsystems/PCGSubsystem.h"


TArray<FPCGPinProperties> UNGetAllJunctionDataSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	
	PinProperties.Emplace(JunctionsAttribute, EPCGDataType::Point);
	
	return PinProperties;
}

FPCGElementPtr UNGetAllJunctionDataSettings::CreateElement() const
{
	return MakeShared<FNGetAllJunctionDataElement>();
}


void FNGetAllJunctionDataElement::GetDependenciesCrc(const FPCGGetDependenciesCrcParams& InParams, FPCGCrc& OutCrc) const
{
	IPCGElement::GetDependenciesCrc(InParams, OutCrc);

	for (UNCellJunctionComponent* JunctionComponent : FNProcGenRegistry::GetCellJunctionComponents())
	{
		if (JunctionComponent)
		{
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.RootRelativeLocation));
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.SocketSize));
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.RootRelativeCardinalRotation.ToIntVector3()));
		}
	}
}

bool FNGetAllJunctionDataElement::ExecuteInternal(FPCGContext* Context) const
{	
	const UNGetAllJunctionDataSettings* Settings = Context->GetInputSettings<UNGetAllJunctionDataSettings>();
	N_PCG_JUNCTION_PREFIX
	for (const UNCellJunctionComponent* JunctionComponent :  FNProcGenRegistry::GetCellJunctionComponents())
	{
		N_PCG_JUNCTION_DATA
	}
	
	FPCGTaggedData& TaggedData = Outputs.Emplace_GetRef();
	TaggedData.Data = OutputData;
	TaggedData.Pin = Settings->JunctionsAttribute; 
	
	return true;
}


#if WITH_EDITOR
void UNGetAllJunctionDataSettings::GetStaticTrackedKeys(FPCGSelectionKeyToSettingsMap& OutKeysToSettings,
	TArray<TObjectPtr<const UPCGGraph>>& OutVisitedGraphs) const
{
	FPCGSettingsAndCulling SettingsAndCulling(TSoftObjectPtr<const UPCGSettings>(this), false);
	
	for (UNCellJunctionComponent* JunctionComponent : FNProcGenRegistry::GetCellJunctionComponents())
	{
		FPCGSelectionKey JunctionKey = FPCGSelectionKey::CreateFromObjectPtr(JunctionComponent);
		
		OutKeysToSettings.FindOrAdd(JunctionKey).AddUnique(SettingsAndCulling);
	}
}
#endif // WITH_EDITOR