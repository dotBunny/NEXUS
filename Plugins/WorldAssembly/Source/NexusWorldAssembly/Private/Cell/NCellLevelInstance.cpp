// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellLevelInstance.h"

#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "Cell/NCellJunctionComponent.h"
#include "Net/UnrealNetwork.h"

ANCellLevelInstance::ANCellLevelInstance()
{
	bReplicates = true;
	
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	
	// Ensure that we are never less than what we consider nearby
	if (HasAuthority())
	{
		const float SquaredNearbyRange = Settings->NetworkNearbyRange * Settings->NetworkNearbyRange;
		if (NetCullDistanceSquared < SquaredNearbyRange)
		{
			SetNetCullDistanceSquared(SquaredNearbyRange);
		}
	}
	
	if (Settings->NetworkingMode == ENWorldAssemblyNetworkMode::AlwaysRelevantLevelInstances)
	{
		bAlwaysRelevant = true;
	}
	
	

}

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromLevel(GetLoadedLevel());
	if (!CellActor)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("No ANCellActor found in level '%s'."),*GetLoadedLevel()->GetName());
		Super::OnLevelInstanceLoaded();
		return;
	}
	CellActor->InitializeFromProxy(this);
	Super::OnLevelInstanceLoaded();
}

void ANCellLevelInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Special
	FDoRepLifetimeParams Params;
	Params.Condition = COND_InitialOnly;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ANCellLevelInstance, AssemblyData, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ANCellLevelInstance, JunctionDetails, Params);
}

void ANCellLevelInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegistered)
	{
		FNWorldAssemblyRegistry::UnregisterCellLevelInstance(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ANCellLevelInstance::OnRep_JunctionDetails()
{
	FillJunctionData();
}

void ANCellLevelInstance::OnRep_AssemblyData()
{
	// This ensures that a cell gets registered on the clients
	FNWorldAssemblyRegistry::RegisterCellLevelInstance(this);
}

void ANCellLevelInstance::FillJunctionData()
{
	JunctionData.Empty();
	for (int32 i = 0; i < JunctionDetails.Num(); i++)
	{
		JunctionData.Add(JunctionDetails[i].InstanceIdentifier, JunctionDetails[i]);
	}
}
