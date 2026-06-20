// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NGetAllJunctionDataElement.h"

#include "NWorldAssemblyPCGMacros.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
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

	for (UNCellJunctionComponent* JunctionComponent : FNWorldAssemblyRegistry::GetCellJunctionComponents())
	{
		if (JunctionComponent)
		{
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.WorldLocation));
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.SocketSize));
			OutCrc.Combine(GetTypeHash(JunctionComponent->Details.WorldRotation.Euler()));
		}
	}
}

bool FNGetAllJunctionDataElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNGetAllJunctionDataSettings* Settings = Context->GetInputSettings<UNGetAllJunctionDataSettings>();
	N_PCG_JUNCTION_PREFIX
	for (const UNCellJunctionComponent* JunctionComponent :  FNWorldAssemblyRegistry::GetCellJunctionComponents())
	{
		if (JunctionComponent == nullptr)
		{
			continue;
		}
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

	for (UNCellJunctionComponent* JunctionComponent : FNWorldAssemblyRegistry::GetCellJunctionComponents())
	{
		FPCGSelectionKey JunctionKey = FPCGSelectionKey::CreateFromObjectPtr(JunctionComponent);

		OutKeysToSettings.FindOrAdd(JunctionKey).AddUnique(SettingsAndCulling);
	}
}
#endif // WITH_EDITOR