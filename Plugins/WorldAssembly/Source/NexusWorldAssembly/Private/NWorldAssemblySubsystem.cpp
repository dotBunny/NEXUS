// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblySubsystem.h"


#include "EngineUtils.h"
#include "NMultiplayerUtils.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyRelay.h"
#include "Cell/INCellJunctionConnector.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellLevelInstance.h"
#include "Cell/NCellProxy.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Math/NMersenneTwister.h"
#include "Developer/NMethodScopeTimer.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Organ/NOrganComponent.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

namespace NEXUS::WorldAssembly::ConsoleCommands
{
	static FAutoConsoleCommand WorldAssemblyRegenerate(TEXT("N.WorldAssembly.Regenerate"),
		TEXT("Regenerates all Organs in a given world, clearing the previous generation."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []
			{
				UWorld* World = FNWorldUtils::GetGameWorld();
				if (World == nullptr)
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to regenerate the world, as the world was NULL."));
					return;
				}

				if (FNMultiplayerUtils::HasWorldAuthority(World))
				{
					UNWorldAssemblySubsystem* System = UNWorldAssemblySubsystem::Get(World);
					System->Clear();

					FNAssemblyOperationSettings Settings = FNAssemblyOperationSettings::GetDefaultSettings();
					System->Generate(Settings);
				}
				else
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to regenerate the world as you do not have authority."));
				}
			});
		}));
	static FAutoConsoleCommand WorldAssemblyClear(TEXT("N.WorldAssembly.Clear"),
		TEXT("Clears the previous generation and any registered cleanup actors."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []
			{
				UWorld* World = FNWorldUtils::GetGameWorld();
				if (World == nullptr)
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to clear the world, as the world was NULL."));
					return;
				}

				if (FNMultiplayerUtils::HasWorldAuthority(World))
				{
					UNWorldAssemblySubsystem* System = UNWorldAssemblySubsystem::Get(World);
					System->Clear();
				}
				else
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to clear the world as you do not have authority."));
				}
			});
		}));
}

void UNWorldAssemblySubsystem::Generate(FNAssemblyOperationSettings& Settings)
{
	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(
		FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Settings);

	StartOperation(Operation);
}

void UNWorldAssemblySubsystem::Clear()
{
	UWorld* World = GetWorld();

	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		if (KnownOperations[i]->IsRunning())
		{
			if (FNAssemblyTaskGraph* Graph = KnownOperations[i]->GetTaskGraph())
			{
				Graph->Cancel();
			}
		}
	}

#if WITH_EDITOR
	// Bulk clears can tear down streamed sublevel actors the user may have selected; drop the entire
	// selection so the typed-element registry does not assert on a stale handle next mouse-move.
	if (GIsEditor && GEditor != nullptr)
	{
		GEditor->SelectNone(false, true, false);
	}
#endif // WITH_EDITOR

	for (TActorIterator<ANCellProxy> It(World, ANCellProxy::StaticClass()); It; ++It)
	{
		ANCellProxy* Proxy = *It;
		Proxy->DestroyLevelInstance(true, true);
		Proxy->Destroy();
	}

	// Drop every connector pairing. The connector actors themselves are tracked per operation and destroyed by the
	// sweep below, so this only discards the bookkeeping — but it has to happen, or a pairing whose cells never
	// streamed in would outlive the generation that produced it and rebuild against the next one.
	PendingJunctionConnectors.Empty();
	QueuedJunctionConnectorsToSpawn.Empty();

	// Handle our track for cleanup — destroy every operation's tracked actors, then drop all buckets.
	for (const TPair<int32, TArray<TWeakObjectPtr<AActor>>>& Pair : TrackedOperationActors)
	{
		DestroyTrackedActors(Pair.Value);
	}
	TrackedOperationActors.Empty();

	// Allow folks to subscribe for this event
	OnCleared.Broadcast();
}

bool UNWorldAssemblySubsystem::IsReady(const bool bWaitOnStreaming)
{
	const UWorld* World = GetWorld();
	if (bWaitOnStreaming && FNWorldUtils::IsStreaming(World))
	{
		return false;
	}

	// Server always has stuff replicated
	if (FNMultiplayerUtils::HasWorldAuthority(World))
	{
		return KnownOperations.IsEmpty();
	}

	// Client hasn't spawned the goodness yet
	if (LocalRelay == nullptr) return false;

	// Client properly checking
	return LocalRelay->IsReady();
}

FIntVector2 UNWorldAssemblySubsystem::GetRemainingStatus()
{
	if (LocalRelay == nullptr)
	{
		return FIntVector2::ZeroValue;
	}

	return LocalRelay->GetRemainingStatus();
}

void UNWorldAssemblySubsystem::DestroyTrackedActors(const TArray<TWeakObjectPtr<AActor>>& Actors)
{
	for (const TWeakObjectPtr<AActor>& Actor : Actors)
	{
		if (Actor.IsValid())
		{
			Actor.Get()->Destroy(true, false);
		}
	}
}

void UNWorldAssemblySubsystem::RegisterOperationActor(AActor* Actor, const int32 OperationTicket)
{
	if (Actor == nullptr) return;
	TrackedOperationActors.FindOrAdd(OperationTicket).AddUnique(Actor);
}

void UNWorldAssemblySubsystem::UnregisterOperationActor(AActor* Actor)
{
	// The actor lives under exactly one ticket, but the caller does not pass it; scan every bucket and prune any
	// that empties so stale ticket keys do not accumulate.
	for (auto It = TrackedOperationActors.CreateIterator(); It; ++It)
	{
		It->Value.Remove(Actor);
		if (It->Value.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

void UNWorldAssemblySubsystem::UnregisterOperationActorByTicket(AActor* Actor, const int32 OperationTicket)
{
	// Direct bucket lookup (unlike UnregisterActorForCleanup, which scans every ticket); prune the bucket when the
	// removal empties it so stale ticket keys do not accumulate.
	if (TArray<TWeakObjectPtr<AActor>>* Bucket = TrackedOperationActors.Find(OperationTicket))
	{
		Bucket->Remove(Actor);
		if (Bucket->IsEmpty())
		{
			TrackedOperationActors.Remove(OperationTicket);
		}
	}
}


void UNWorldAssemblySubsystem::DestroyOperationActors(const int32 OperationTicket)
{
	TArray<TWeakObjectPtr<AActor>> Bucket;
	if (TrackedOperationActors.RemoveAndCopyValue(OperationTicket, Bucket))
	{
		DestroyTrackedActors(Bucket);
	}
}

void UNWorldAssemblySubsystem::Tick(float DeltaTime)
{
	// Seamless-traveled players arrive asynchronously through HandleSeamlessTravelPlayer (no delegate to bind), so
	// when seamless travel is supported we poll for unserved controllers. Throttled to roughly once per half-second: a
	// frame-accurate response buys nothing and SpawnRelay is idempotent, so missed frames cost only a little latency.
	if (bCachedSeamlessTravelMonitor)
	{
		SeamlessTravelMonitorAccumulator += DeltaTime;
		if (SeamlessTravelMonitorAccumulator >= SeamlessTravelMonitorInterval)
		{
			SeamlessTravelMonitorAccumulator = 0.f;
			EnsurePlayerControllerRelays(GetWorld());
		}
	}

	if (KnownOperations.Num() > 0)
	{
		for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
		{
			KnownOperations[i]->Tick();
		}
	}

	// If we have anything queued for generation, lets get it going
	if (QueuedOrgansForAssembly.Num() > 0)
	{
		if (!FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
		{
			QueuedOrgansForAssembly.Empty();
		}
		else
		{
			FNAssemblyOperationSettings Settings = FNAssemblyOperationSettings::GetDefaultSettings();
			Settings.DisplayName = FText::FromString("QueuedOrganAssembly");
			UNAssemblyOperation* InstanceOperation = UNAssemblyOperation::CreateInstance(QueuedOrgansForAssembly, Settings);

			StartOperation(InstanceOperation);

			// We issue the generation and then clear the queue.
			QueuedOrgansForAssembly.Empty();
		}
	}

	// Handle spawning / filling junctions, time-sliced so a large backlog
	// drains across multiple ticks instead of stalling one frame.
	if (QueuedCellJunctionsToFill.Num() > 0)
	{
		// Setting is authored in milliseconds; convert to seconds for the timer.
		const double SliceSeconds = CachedCellJunctionTimeSlice;
		const double StartTime = FPlatformTime::Seconds();
		for (int32 i = QueuedCellJunctionsToFill.Num() - 1; i >= 0; i--)
		{
			if (UNCellJunctionComponent* Junction = QueuedCellJunctionsToFill[i])
			{
				Junction->Fill();
			}
			QueuedCellJunctionsToFill.RemoveAt(i);

			// Always fill at least one per tick; stop once the slice is spent.
			if (FPlatformTime::Seconds() - StartTime > SliceSeconds)
			{
				break;
			}
		}
	}

	// Connector pairings whose two cells have both streamed in. Shares the junction slice budget: both are the same
	// kind of work (spawning a piece of geometry into a junction) arriving on the same streaming-driven schedule.
	if (QueuedJunctionConnectorsToSpawn.Num() > 0)
	{
		const double SliceSeconds = CachedCellJunctionTimeSlice;
		const double StartTime = FPlatformTime::Seconds();
		for (int32 i = QueuedJunctionConnectorsToSpawn.Num() - 1; i >= 0; i--)
		{
			const int32 ConnectorIdentifier = QueuedJunctionConnectorsToSpawn[i];
			QueuedJunctionConnectorsToSpawn.RemoveAt(i);

			if (FPendingJunctionConnector* Pending = PendingJunctionConnectors.Find(ConnectorIdentifier))
			{
				Pending->bQueued = false;
				SpawnJunctionConnector(*Pending);
			}

			// Always spawn at least one per tick; stop once the slice is spent.
			if (FPlatformTime::Seconds() - StartTime > SliceSeconds)
			{
				break;
			}
		}
	}
}

bool UNWorldAssemblySubsystem::IsTickable() const
{
	if (KnownOperations.Num() > 0 || QueuedOrgansForAssembly.Num() > 0  || QueuedCellJunctionsToFill.Num() > 0
		|| QueuedJunctionConnectorsToSpawn.Num() > 0 || bCachedSeamlessTravelMonitor) return true;
	return false;
}

void UNWorldAssemblySubsystem::StartOperation(UNAssemblyOperation* Operation)
{
	if (Operation == nullptr) return;

	KnownOperations.AddUnique(Operation);
	OnOperationStarted.Broadcast();
	CachedOperationTickets.Add(Operation->GetTicket());

	Operation->StartBuild(this, this);

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationStarted(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	const int RemoveCount = KnownOperations.Remove(Operation);
	if (RemoveCount > 0 && KnownOperations.IsEmpty())
	{
		OnOperationsCompleted.Broadcast();
	}

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationFinished(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::OnOperationDestroyed(UNAssemblyOperation* Operation)
{
	const int RemoveCount = KnownOperations.Remove(Operation);
	if (RemoveCount > 0 && KnownOperations.IsEmpty())
	{
		OnOperationsCompleted.Broadcast();
	}

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationDestroyed(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::RegisterLocalRelay(ANWorldAssemblyRelay* InRelay)
{
	LocalRelay = InRelay;
}

void UNWorldAssemblySubsystem::UnregisterLocalRelay(const ANWorldAssemblyRelay* InRelay)
{
	if (LocalRelay == InRelay)
	{
		LocalRelay = nullptr;
	}
}

void UNWorldAssemblySubsystem::RegisterOrganForAssembly(TObjectPtr<UNOrganComponent> Organ)
{

	if (!FNMultiplayerUtils::HasWorldAuthority(GetWorld())) return;
	QueuedOrgansForAssembly.AddUnique(Organ);
}

void UNWorldAssemblySubsystem::RegisterCellJunctionToFill(TObjectPtr<UNCellJunctionComponent> CellJunction)
{
	// TODO: Do we want to make cell filling server authoritative? This will need some changes down the line, nothing major.
	QueuedCellJunctionsToFill.AddUnique(CellJunction);
}

void UNWorldAssemblySubsystem::RegisterPendingJunctionConnector(const FNCellJunctionConnection& Connection)
{
	if (Connection.ConnectorIdentifier == INDEX_NONE) return;

	FPendingJunctionConnector& Pending = PendingJunctionConnectors.FindOrAdd(Connection.ConnectorIdentifier);
	Pending.Connection = Connection;
}

void UNWorldAssemblySubsystem::RegisterJunctionConnectorEndpoint(UNCellJunctionComponent* CellJunction)
{
	if (CellJunction == nullptr) return;

	const FNCellLinkDetails& LinkDetails = CellJunction->LinkDetails;
	if (!LinkDetails.bConnector) return;

	FPendingJunctionConnector* Pending = PendingJunctionConnectors.Find(LinkDetails.ConnectorIdentifier);
	if (Pending == nullptr)
	{
		// The endpoint arrived without a pairing to join. Expected on clients, where the junction's link details
		// replicate with the cell but the pass that produced the pairing only ever ran on the server.
		return;
	}

	// Match on the junction's own identifier rather than on arrival order: both cells could carry the same node
	// identifier (they restart per graph), so the junction instance is what distinguishes the two ends.
	if (LinkDetails.JunctionInstanceIdentifier == Pending->Connection.StartJunctionInstanceIdentifier
		&& LinkDetails.NodeIdentifier == Pending->Connection.StartNodeIdentifier)
	{
		Pending->StartJunction = CellJunction;
	}
	else
	{
		Pending->EndJunction = CellJunction;
	}

	// Queue only once both ends are live and nothing has been built yet; a cell streaming back in re-reports, and
	// the bQueued guard is what stops that enqueuing the same pairing twice.
	if (Pending->bQueued || Pending->ConnectorActor.IsValid()) return;
	if (!Pending->StartJunction.IsValid() || !Pending->EndJunction.IsValid()) return;

	Pending->bQueued = true;
	QueuedJunctionConnectorsToSpawn.AddUnique(LinkDetails.ConnectorIdentifier);
}

void UNWorldAssemblySubsystem::UnregisterJunctionConnectorEndpoint(const UNCellJunctionComponent* CellJunction)
{
	if (CellJunction == nullptr) return;

	const FNCellLinkDetails& LinkDetails = CellJunction->LinkDetails;
	if (!LinkDetails.bConnector) return;

	FPendingJunctionConnector* Pending = PendingJunctionConnectors.Find(LinkDetails.ConnectorIdentifier);
	if (Pending == nullptr) return;

	if (Pending->StartJunction.Get() == CellJunction)
	{
		Pending->StartJunction.Reset();
	}
	else if (Pending->EndJunction.Get() == CellJunction)
	{
		Pending->EndJunction.Reset();
	}
	else
	{
		return;
	}

	// A connector spans two cells, so losing either end leaves it bridging to nothing. Tear it down and drop out of
	// the queue; the pairing itself is kept, so streaming the cell back in rebuilds it.
	if (AActor* ConnectorActor = Pending->ConnectorActor.Get())
	{
		UnregisterOperationActorByTicket(ConnectorActor, Pending->Connection.OperationTicket);
		ConnectorActor->Destroy();
	}
	Pending->ConnectorActor.Reset();

	if (Pending->bQueued)
	{
		Pending->bQueued = false;
		QueuedJunctionConnectorsToSpawn.Remove(LinkDetails.ConnectorIdentifier);
	}
}

const FNCellJunctionConnectorEntry* UNWorldAssemblySubsystem::SelectJunctionConnectorEntry(
	const TArray<FNCellJunctionConnectorEntry>& Entries, ANCellLevelInstance* CellLevelInstance, const int32 ConnectorIdentifier)
{
	if (Entries.IsEmpty() || CellLevelInstance == nullptr) return nullptr;

	const FNCellAssemblyData& AssemblyData = CellLevelInstance->GetAssemblyData();
	const FNGameplayTagCounter TagCounter(AssemblyData.TagCounter);

	// Gated exactly as fillers are (see UNCellJunctionComponent::GetJunctionFillEntries), so an author who knows one
	// list knows the other.
	FNWeightedIntegerArray WeightedIndices;
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FNCellJunctionConnectorEntry& Entry = Entries[i];
		if (Entry.Actor == nullptr) continue;

		// REQUIRED CONTEXT TAGS — the cell's resolved context must satisfy every tag the entry requires.
		if (!Entry.RequiredContextTags.IsEmpty() && !AssemblyData.ContextTags.HasAllExact(Entry.RequiredContextTags))
		{
			continue;
		}

		// TAG COUNTER CONSTRAINTS — every constraint must pass; an untracked tag compares as a count of zero.
		bool bGatedByTagCounter = false;
		for (const FNGameplayTagCounterConstraint& Constraint : Entry.TagCounterConstraints)
		{
			if (!Constraint.DoesPassComparison(TagCounter))
			{
				bGatedByTagCounter = true;
				break;
			}
		}
		if (bGatedByTagCounter) continue;

		WeightedIndices.Add(i, Entry.Weighting);
	}

	// Seeded from the cell's own seed plus the pairing, so the choice is stable across runs and a junction paired
	// twice in different layouts does not always land on the same entry.
	FNMersenneTwister RandomGenerator(AssemblyData.Seed ^ AssemblyData.NodeIdentifier ^ ConnectorIdentifier);
	const int32 EntryIndex = WeightedIndices.TwistedValue(RandomGenerator);
	return EntryIndex != INDEX_NONE ? &Entries[EntryIndex] : nullptr;
}

UNOrganComponent* UNWorldAssemblySubsystem::FindOrganComponent(const FGuid& Identifier)
{
	if (!Identifier.IsValid()) return nullptr;

	for (UNOrganComponent* Organ : FNWorldAssemblyRegistry::GetOrganComponents())
	{
		if (IsValid(Organ) && Organ->Identifier == Identifier)
		{
			return Organ;
		}
	}
	return nullptr;
}

UClass* UNWorldAssemblySubsystem::ResolveJunctionConnectorClass(const FPendingJunctionConnector& Pending, FTransform& OutOffset) const
{
	OutOffset = FTransform::Identity;

	UNCellJunctionComponent* StartJunction = Pending.StartJunction.Get();
	UNCellJunctionComponent* EndJunction = Pending.EndJunction.Get();
	if (StartJunction == nullptr || EndJunction == nullptr) return nullptr;

	ANCellLevelInstance* StartCell = Cast<ANCellLevelInstance>(StartJunction->GetLevelInstance());
	ANCellLevelInstance* EndCell = Cast<ANCellLevelInstance>(EndJunction->GetLevelInstance());
	const int32 ConnectorIdentifier = Pending.Connection.ConnectorIdentifier;

	// Priority chain, most specific first. The start end wins over the end at each level, which is what makes the
	// pass's deterministic start/end ordering the tiebreak between two junctions that both name a connector.
	const FNCellJunctionConnectorEntry* Entry = SelectJunctionConnectorEntry(StartJunction->Connectors, StartCell, ConnectorIdentifier);
	if (Entry == nullptr)
	{
		Entry = SelectJunctionConnectorEntry(EndJunction->Connectors, EndCell, ConnectorIdentifier);
	}
	if (Entry == nullptr)
	{
		if (const UNOrganComponent* Organ = FindOrganComponent(Pending.Connection.StartOrganIdentifier))
		{
			Entry = SelectJunctionConnectorEntry(Organ->Connectors, StartCell, ConnectorIdentifier);
		}
	}
	if (Entry == nullptr)
	{
		if (const UNOrganComponent* Organ = FindOrganComponent(Pending.Connection.EndOrganIdentifier))
		{
			Entry = SelectJunctionConnectorEntry(Organ->Connectors, EndCell, ConnectorIdentifier);
		}
	}

	if (Entry != nullptr)
	{
		// Authored entries hard-reference their class, so it is always resident and can spawn synchronously.
		OutOffset = Entry->Offset;
		return Entry->Actor;
	}

	// Nothing authored: fall back to the project-wide default. It is a soft reference, and the spawn task asked for
	// it to be loaded when the pairings were registered — if that has not landed yet, Get() returns null and the
	// pairing is dropped rather than stalling the queue. In practice cells take far longer to stream than the class.
	const TSoftClassPtr<AActor>& DefaultConnector = UNWorldAssemblySettings::Get()->AssemblyDefaultJunctionConnector;
	return DefaultConnector.IsNull() ? nullptr : DefaultConnector.Get();
}

void UNWorldAssemblySubsystem::SpawnJunctionConnector(FPendingJunctionConnector& Pending)
{
	UNCellJunctionComponent* StartJunction = Pending.StartJunction.Get();
	UNCellJunctionComponent* EndJunction = Pending.EndJunction.Get();
	if (StartJunction == nullptr || EndJunction == nullptr) return;

	FTransform Offset;
	UClass* ConnectorClass = ResolveJunctionConnectorClass(Pending, Offset);
	if (ConnectorClass == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("Unable to connect junctions for connector %i as no connector was available; the junctions stay open."),
			Pending.Connection.ConnectorIdentifier);
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = StartJunction->GetOwner();
	SpawnParams.OverrideLevel = StartJunction->GetComponentLevel();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	// Anchored on the start junction so the connector's own frame lines up with the route it is handed, whose points
	// are world-space. The authored offset is applied in that junction's frame, matching how fillers are placed.
	const FQuat ConnectorRotation = StartJunction->GetComponentRotation().Quaternion();
	AActor* SpawnedActor = World->SpawnActor<AActor>(ConnectorClass,
		StartJunction->GetComponentLocation() + ConnectorRotation.RotateVector(Offset.GetLocation()),
		(ConnectorRotation * FQuat(Offset.Rotator())).Rotator(),
		SpawnParams);

	if (SpawnedActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to spawn connector %i."), Pending.Connection.ConnectorIdentifier);
		return;
	}

	SpawnedActor->SetActorScale3D(SpawnedActor->GetActorScale3D() * Offset.GetScale3D());

	RegisterOperationActor(SpawnedActor, Pending.Connection.OperationTicket);
	Pending.ConnectorActor = SpawnedActor;

	if (SpawnedActor->Implements<UNCellJunctionConnector>())
	{
		INCellJunctionConnector::Execute_OnConnectJunctions(SpawnedActor,
			Cast<ANCellLevelInstance>(StartJunction->GetLevelInstance()), StartJunction,
			Pending.Connection.StartJunctionInstanceIdentifier,
			Cast<ANCellLevelInstance>(EndJunction->GetLevelInstance()), EndJunction,
			Pending.Connection.EndJunctionInstanceIdentifier,
			Pending.Connection.Path);
	}
	else
	{
		UE_LOG(LogNexusWorldAssembly, Warning,
			TEXT("Unable to invoke OnConnectJunctions on %s as it does not implement the INCellJunctionConnector."),
			*SpawnedActor->GetName())
	}
}

void UNWorldAssemblySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	// Only do this on the server
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	bCachedSeamlessTravelMonitor = Settings->bSupportSeamlessTravel;
	CachedCellJunctionTimeSlice = Settings->AssemblySpawningDelayedJunctionSpawningTimeSlice * 0.001f;

	if (FNMultiplayerUtils::HasWorldAuthority(InWorld))
	{
		// Login/logout cover fresh connections and late joins in every mode; they never fire for players carried
		// across by seamless travel, so binding them unconditionally is safe and never double-spawns a relay.
		OnLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UNWorldAssemblySubsystem::OnPostLogin);
		OnLogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &UNWorldAssemblySubsystem::OnLogout);

		// One-shot back-fill for controllers already present when this world begins play — notably the listen-server
		// local PC, which the engine hands to HandleSeamlessTravelPlayer before BeginPlay. Remote clients that finish
		// loading later are picked up by the throttled monitor in Tick when seamless travel is enabled.
		EnsurePlayerControllerRelays(InWorld.GetWorld());
	}
	Super::OnWorldBeginPlay(InWorld);
}

void UNWorldAssemblySubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	RelayMap.Reset();
	TrackedOperationActors.Empty();
	LocalRelay = nullptr;

	// Stop all known operations
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		UNAssemblyOperation* Operation = KnownOperations[i];
		if (Operation->IsRunning())
		{
			Operation->Cancel();
		}
		else
		{
			Operation->TearDownOperation();
		}
	}

	// Clear cached persistent operation data
	if (CachedOperationTickets.Num() > 0)
	{
		FNWorldAssemblyContextCache::ClearContext(CachedOperationTickets);
		CachedOperationTickets.Empty();
	}

	if (OnLoginHandle.IsValid())
	{
		FGameModeEvents::GameModePostLoginEvent.Remove(OnLoginHandle);
		OnLoginHandle.Reset();
	}
	if (OnLogoutHandle.IsValid())
	{
		FGameModeEvents::GameModeLogoutEvent.Remove(OnLogoutHandle);
		OnLogoutHandle.Reset();
	}

	Super::OnWorldEndPlay(InWorld);
}

void UNWorldAssemblySubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	SpawnRelay(NewPlayer);
}

void UNWorldAssemblySubsystem::OnLogout(AGameModeBase* GameMode, AController* Exiting)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		DestroyRelay(PC);
	}
}

void UNWorldAssemblySubsystem::SpawnRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	if (RelayMap.Contains(PlayerController)) return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = PlayerController;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANWorldAssemblyRelay* NewRelay = GetWorld()->SpawnActor<ANWorldAssemblyRelay>(ANWorldAssemblyRelay::StaticClass(), SpawnInfo);
	if (NewRelay == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to spawn ANWorldAssemblyRelay for PlayerController(%s)."), *PlayerController->GetName());
		return;
	}

	RelayMap.Add(PlayerController, NewRelay);
}

void UNWorldAssemblySubsystem::DestroyRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	TObjectPtr<ANWorldAssemblyRelay> Existing;
	if (RelayMap.RemoveAndCopyValue(PlayerController, Existing) && Existing != nullptr)
	{
		Existing->Destroy();
	}
}

void UNWorldAssemblySubsystem::EnsurePlayerControllerRelays(const UWorld* World)
{
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			SpawnRelay(PC);
		}
	}
}
