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
	const float SquaredNearbyRange = Settings->NetworkNearbyRange * Settings->NetworkNearbyRange;
	if (GetNetCullDistanceSquared() < SquaredNearbyRange)
	{
		SetNetCullDistanceSquared(SquaredNearbyRange);
	}

	if (Settings->NetworkingMode == ENWorldAssemblyNetworkMode::AlwaysRelevantLevelInstances)
	{
		bAlwaysRelevant = true;
	}
}

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	const ULevel* LoadedLevel = GetLoadedLevel();
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromLevel(LoadedLevel);
	if (!CellActor)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("No ANCellActor found in level '%s'."),
			LoadedLevel ? *LoadedLevel->GetName() : TEXT("null"));
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
}

void ANCellLevelInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegistered)
	{
		FNWorldAssemblyRegistry::UnregisterCellLevelInstance(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ANCellLevelInstance::BeginDestroy()
{
	// Guaranteed-before-free backstop for the teardown paths that never route EndPlay (editor undo, forced
	// delete, GC of a never-played actor). UnregisterCellLevelInstance clears bRegistered, so if EndPlay
	// already ran this is a no-op and never double-unregisters.
	if (bRegistered)
	{
		FNWorldAssemblyRegistry::UnregisterCellLevelInstance(this);
	}
	Super::BeginDestroy();
}



void ANCellLevelInstance::OnRep_AssemblyData()
{
	// This ensures that a cell gets registered on the clients
	FNWorldAssemblyRegistry::RegisterCellLevelInstance(this);

	// Apply updates
	UpdateFromAssemblyData();
}

void ANCellLevelInstance::UpdateFromAssemblyData()
{
	JunctionData.Empty();
	for (int32 i = 0; i < AssemblyData.JunctionDetails.Num(); i++)
	{
		JunctionData.Add(AssemblyData.JunctionDetails[i].InstanceIdentifier, AssemblyData.JunctionDetails[i]);
	}
}

FNCellLinkDetails ANCellLevelInstance::GetCellLinkDetails(const int32 JunctionIdentifier)
{
	if (AssemblyData.LinkDetails.Num() == 0)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Junction(%i) has requested link details prior to it being replicated."), JunctionIdentifier);
		return FNCellLinkDetails();
	}

	for (int32 i = 0; i < AssemblyData.LinkDetails.Num(); i++)
	{
		const FNCellLinkDetails& LinkDetails = AssemblyData.LinkDetails[i];
		if (LinkDetails.JunctionInstanceIdentifier == JunctionIdentifier)
		{
			return LinkDetails;
		}
	}

	UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Junction(%i) has requested link details that couldn't be found."), JunctionIdentifier);
	return FNCellLinkDetails();
}
