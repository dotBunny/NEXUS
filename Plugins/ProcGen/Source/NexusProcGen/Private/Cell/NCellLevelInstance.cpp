// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellLevelInstance.h"

#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Net/UnrealNetwork.h"

ANCellLevelInstance::ANCellLevelInstance()
{
	bReplicates = true;
	if (UNProcGenSettings::Get()->NetworkingMode == ENProcGenNetworkMode::AlwaysRelevantLevelInstances)
	{
		bAlwaysRelevant = true;
	}
}

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	ANCellActor* CellActor = FNProcGenUtils::GetCellActorFromLevel(GetLoadedLevel());
	CellActor->InitializeFromProxy(this);
	Super::OnLevelInstanceLoaded();
}

void ANCellLevelInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Special
	FDoRepLifetimeParams Params;
	Params.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ANCellLevelInstance, OperationTicket, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ANCellLevelInstance, JunctionDetails, Params);
}

void ANCellLevelInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FNProcGenRegistry::UnregisterCellLevelInstance(this);
	Super::EndPlay(EndPlayReason);
}

void ANCellLevelInstance::OnRep_JunctionDetails()
{
	FillJunctionData();
}

void ANCellLevelInstance::FillJunctionData()
{
	JunctionData.Empty();
	for (int32 i = 0; i < JunctionDetails.Num(); i++)
	{
		JunctionData.Add(JunctionDetails[i].InstanceIdentifier, JunctionDetails[i]);
	}
}
