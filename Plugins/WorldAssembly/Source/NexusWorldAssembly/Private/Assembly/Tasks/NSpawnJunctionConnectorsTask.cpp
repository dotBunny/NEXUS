// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NSpawnJunctionConnectorsTask.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblySubsystem.h"
#include "Engine/AssetManager.h"

FNSpawnJunctionConnectorsTask::FNSpawnJunctionConnectorsTask(
	const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
}

void FNSpawnJunctionConnectorsTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	const TArray<FNCellJunctionConnection>& Connections = TaskGraphContextPtr->JunctionConnections;
	if (Connections.IsEmpty() || TaskGraphContextPtr->IsCancelled())
	{
		return;
	}

	// Without level instances there are no junction components for a connector to attach to, so no pairing could
	// ever complete. Registering them anyway would leave the subsystem holding entries for the rest of the session.
	if (!TaskGraphContextPtr->OperationSettings.bCreateLevelInstances)
	{
		UE_LOG(LogNexusWorldAssembly, Log,
			TEXT("Skipping %i junction connectors: this operation does not create level instances, so the junctions they would connect are never spawned."),
			Connections.Num());
		return;
	}

	UWorld* TargetWorld = TaskGraphContextPtr->TargetWorld;
	if (!IsValid(TargetWorld) || TargetWorld->bIsTearingDown)
	{
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("Skipping FNSpawnJunctionConnectorsTask as the World is in a bad state (PIE, teardown, etc.)."));
		return;
	}

	UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(TargetWorld);
	if (Subsystem == nullptr)
	{
		return;
	}

	for (const FNCellJunctionConnection& Connection : Connections)
	{
		Subsystem->RegisterPendingJunctionConnector(Connection);
	}

	// Start the default connector loading now rather than when the first pairing completes. It is a soft reference,
	// and cells take far longer to stream than a class does to load, so by the time any pairing is ready this has
	// almost always landed — which keeps the spawn path synchronous.
	const TSoftClassPtr<AActor>& DefaultConnector = UNWorldAssemblySettings::Get()->AssemblyDefaultJunctionConnector;
	if (!DefaultConnector.IsNull() && DefaultConnector.Get() == nullptr)
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(DefaultConnector.ToSoftObjectPath());
	}

	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Registered %i junction connectors awaiting their cells."), Connections.Num());
}
