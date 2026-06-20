// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPool.h"
#include "INActorPoolItem.h"
#include "NActorPoolObject.h"
#include "NActorPoolsSettings.h"
#include "NActorPoolSubsystem.h"
#include "NActorUtils.h"
#include "NActorPoolsMinimal.h"

#if WITH_EDITOR
int32 FNActorPool::ActorPoolTicket = 0;
#endif // WITH_EDITOR

FNActorPool::FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass)
{
	PreInitialize(TargetWorld, ActorClass);
	if (bImplementsPoolItemInterface)
	{
		AActor* DefaultActor = ActorClass->GetDefaultObject<AActor>();
		UpdateSettings(Cast<INActorPoolItem>(DefaultActor)->GetActorPoolSettings());
	}
	else
	{
		UpdateSettings(UNActorPoolsSettings::Get()->DefaultSettings);
	}
	PostInitialize();
}

FNActorPool::FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass, const FNActorPoolSettings& InActorPoolSetting)
{
	PreInitialize(TargetWorld, ActorClass);
	UpdateSettings(InActorPoolSetting);
	PostInitialize();
}

FNActorPool::~FNActorPool()
{
	// The world creates and tears down the pools, so we probably dont need to remove it from tickable.
	
	if (IsValid(LinkedActorPoolObject))
	{
		LinkedActorPoolObject->Pool = nullptr;
	}
}

FText FNActorPool::GetDescription() const
{
	// World Name
	FString WorldDetails = TEXT("WORLD:\n");
	if (World != nullptr)
	{
		WorldDetails += "\t" + World->GetName() + TEXT("\n");
	}
	else
	{
		WorldDetails += TEXT("NULL\n");
	}
	
	// Actor Details
	FString ActorDetails = TEXT("ACTOR:\n");
#if WITH_EDITOR
	ActorDetails += TEXT("\tSpawn Prefix: ") + Name + TEXT("\n");
#endif
	if (Template != nullptr)
	{
		ActorDetails += FString::Printf(TEXT("\tTemplate Name: %s\n"), *Template->GetName());
		ActorDetails += FString::Printf(TEXT("\tTemplate Ptr: %p\n"), Template.Get());
	}
	else
	{
		ActorDetails += TEXT("\tTemplate: NULL\n");
	}
	
	switch (Settings.Strategy)
	{
		using enum ENActorPoolStrategy;
	case Create:
		ActorDetails += TEXT("\tStrategy: Create\n");
		break;
	case CreateLimited:
		ActorDetails += TEXT("\tStrategy: Create Limited\n");
		break;
	case CreateRecycleFirst:
		ActorDetails += TEXT("\tStrategy: Create Recycle First\n");
		break;
	case CreateRecycleLast:
		ActorDetails += TEXT("\tStrategy: Create Recycle Last\n");
		break;
	case Fixed:
		ActorDetails += TEXT("\tStrategy: Fixed\n");
		break;
	case FixedRecycleFirst:
		ActorDetails += TEXT("\tStrategy: Fixed Recycle First\n");
		break;
	case FixedRecycleLast:
		ActorDetails += TEXT("\tStrategy: Fixed Recycle Last\n");
		break;
	}
	ActorDetails += FString::Printf(TEXT("\tSpawn Physics Simulation: %s\n"), *FNToggle::ToString(SpawnPhysicsSimulation));
	
	// Flags
	FString FlagDetails = TEXT("FLAGS:\n");
	if (Settings.HasFlag_BroadcastRelease())
	{
		FlagDetails += TEXT("\tBroadcast Release\n");
	}
	if (Settings.HasFlag_InvokeUFunctions())
	{
		FlagDetails += TEXT("\tInvoke UFunctions\n");
	}
	if (Settings.HasFlag_ReturnToStorage())
	{
		FlagDetails += TEXT("\tReturn To Storage\n");
	}
	if (Settings.HasFlag_DeferConstruction())
	{
		FlagDetails += TEXT("\tDefer Construction\n");
	}
	if (Settings.HasFlag_ServerOnly())
	{
		FlagDetails += TEXT("\tServer Only\n");
	}
	if (Settings.HasFlag_SetNetDormancy())
	{
		FlagDetails += TEXT("\tSet Net Dormancy\n");
	}
	if (Settings.HasFlag_SweepBeforeSettingLocation())
	{
		FlagDetails += TEXT("\tSweep Before Setting Location\n");
	}
	
	return FText::Format(NSLOCTEXT("NexusActorPools", "ActorPoolDescription", "{0}\n{1}\n{2}"), 
		FText::FromString(WorldDetails), FText::FromString(ActorDetails), FText::FromString(FlagDetails));
}

void FNActorPool::PreInitialize(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass)
{
	World = TargetWorld;
	Template = ActorClass;
	if (!ensureMsgf(Template != nullptr, TEXT("Preinitialized ActorPool with a null actor class. Pool will not be usable!")))
	{
		return;
	}
	
	USceneComponent* RootComponent = FNActorUtils::GetRootComponentFromDefaultObject(ActorClass);
	bImplementsPoolItemInterface = Template->ImplementsInterface(UNActorPoolItem::StaticClass());

	// The ActorPool system requires that the root component of its actors be used to determine the physics settings.
	if (RootComponent != nullptr)
	{
		if (const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(RootComponent))
		{
			if (PrimitiveComponent->BodyInstance.bSimulatePhysics)
			{
				SpawnPhysicsSimulation = ENToggle::Enabled;
			}
			else
			{
				SpawnPhysicsSimulation = ENToggle::Disabled;
			}
		}
	}

#if WITH_EDITOR
	// Increments only on the game thread, no real issue with roll over. 
	// If you have that many pools you might have a different problems to answer.
	ActorPoolTicket++;

	// Generate the name for the actor pool
	FString WorkingName = Template->GetName();
	if (WorkingName.EndsWith(TEXT("_C")))
	{
		WorkingName = WorkingName.Mid(0, WorkingName.Len() - 2);
	}
	Name = FString::Printf(TEXT("AP_%s_%i"), *WorkingName, ActorPoolTicket);
#endif // WITH_EDITOR
}

void FNActorPool::PostInitialize()
{
	if (Settings.CreateObjectsPerTick == -1)
	{
		Fill();
	}
}

AActor* FNActorPool::Get()
{
	if (InActors.IsEmpty() && !ApplyStrategy())
	{
		return nullptr;
	}

	// ApplyStrategy() reporting success implies a poppable actor for every strategy today, but that contract is
	// implicit — a future strategy that batches creation could break it. ensure() makes the invariant loud at
	// dev time while keeping Shipping safe from an empty-array Pop().
	if (!ensure(!InActors.IsEmpty()))
	{
		return nullptr;
	}

	AActor* ReturnActor = InActors.Pop();
	OutActors.Add(ReturnActor);
	return ReturnActor;
}

AActor* FNActorPool::Spawn(const FVector& Position, const FRotator& Rotation)
{
	if (InActors.IsEmpty() && !ApplyStrategy())
	{
		return nullptr;
	}

	if (!ensure(!InActors.IsEmpty()))
	{
		return nullptr;
	}

	AActor* ReturnActor = InActors.Pop();
	OutActors.Add(ReturnActor);
	ApplySpawnState(ReturnActor, Position, Rotation);
	
	if (bImplementsPoolItemInterface)
	{
		(Cast<INActorPoolItem>(ReturnActor))->OnSpawnedFromActorPool();
	}
	else if (Settings.HasFlag_InvokeUFunctions()) // SLOW PATH
	{
		UFunction* Function = ReturnActor->FindFunction(NEXUS::ActorPools::InvokeMethods::OnSpawnedFromActorPool);
		if (Function)
		{
			ReturnActor->ProcessEvent(Function, nullptr);
		}
	}

	return ReturnActor;
}

bool FNActorPool::Return(AActor* Actor)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return true;

	if (Actor == nullptr)
	{
		UE_LOG(LogNexusActorPools, Warning, TEXT("Attempted to return a NULL reference to a FNActorPool."));
		return false;
	}
#if !UE_BUILD_SHIPPING
	// Development-only contract checks — stripped in Shipping for hot-path cost; callers in Shipping are expected to honour these invariants.

	// Mismatched-class actors would pollute InActors; a future Get() would Cast<T> them to the wrong type.
	if (Actor->GetClass() != Template)
	{
		UE_LOG(LogNexusActorPools, Warning,
			TEXT("Attempted to return an actor of class %s to a FNActorPool templated on %s; rejecting."),
			*Actor->GetClass()->GetName(), *Template->GetName());
		return false;
	}

	// Re-adding an actor already in the pool would hand the same pointer back to multiple callers from subsequent Get()/Spawn() calls.
	if (InActors.Contains(Actor))
	{
		UE_LOG(LogNexusActorPools, Warning,
			TEXT("Attempted to return an actor (%s) that is already pooled in a FNActorPool; rejecting double-add."),
			*Actor->GetName());
		return false;
	}
#endif // !UE_BUILD_SHIPPING
	
	ApplyReturnState(Actor);

	// We have to manage the position a bit based on the strategy.
	switch (Settings.Strategy)
	{
		using enum ENActorPoolStrategy;
	case CreateRecycleFirst:
	case CreateRecycleLast:
	case FixedRecycleFirst:
	case FixedRecycleLast:
		OutActors.Remove(Actor);
		break;
	default:
		OutActors.RemoveSwap(Actor, EAllowShrinking::No);
		break;
	}
	
	FinalizeReturn(Actor);
	return true;
}

bool FNActorPool::ReturnAtIndex(const int32 OutIndex)
{
	// Recycle-only fast path. ApplyStrategy already located the element being recycled, so we remove
	// it by its known index instead of making Return() re-scan OutActors for a pointer it was just
	// handed (and skip the external-caller contract checks, which a self-recycle always satisfies).
	// RemoveAt keeps the surviving elements ordered, preserving the FIFO/LIFO semantics the recycle
	// strategies depend on; EAllowShrinking::No matches Return() and avoids a realloc on this
	// (saturated-pool) hot path.
	AActor* Actor = OutActors[OutIndex];
	if (Actor == nullptr)
	{
		UE_LOG(LogNexusActorPools, Warning, TEXT("Attempted to recycle a NULL reference from a FNActorPool."));
		return false;
	}

	ApplyReturnState(Actor);
	OutActors.RemoveAt(OutIndex, EAllowShrinking::No);
	FinalizeReturn(Actor);
	return true;
}

void FNActorPool::FinalizeReturn(AActor* Actor)
{
	InActors.Add(Actor);

	if (bImplementsPoolItemInterface)
	{
		(Cast<INActorPoolItem>(Actor))->OnReturnToActorPool();
	}
	else if (Settings.HasFlag_InvokeUFunctions()) // SLOW PATH
	{
		UFunction* Function = Actor->FindFunction(NEXUS::ActorPools::InvokeMethods::OnReturnToActorPool);
		if (Function)
		{
			Actor->ProcessEvent(Function, nullptr);
		}
	}
}

void FNActorPool::ReturnAll(bool bSkipCheck)
{
	// Stub pools never populate OutActors; mirror Return()'s short-circuit for symmetry.
	if (bStubMode) return;

	if (!bSkipCheck && !Settings.HasSupportFlag_ReturnAll())
	{
		UE_LOG(LogNexusActorPools, Warning, TEXT("ReturnAll called on a FNActorPool(%s) that does not support it."), *Template->GetName());
		return;
	}
	
	for (int i = OutActors.Num() - 1; i >= 0; --i)
	{
		Return(OutActors[i]);
	}
}

void FNActorPool::UpdateSettings(const FNActorPoolSettings& InNewSettings)
{
	// Ingest flags - and update cached flags
	Settings.Flags = InNewSettings.Flags;
	Settings.SupportFlags = InNewSettings.SupportFlags;
	
	// World may have been torn down out from under a long-lived external pool reference; treat that as non-authoritative.
	bStubMode = Settings.HasFlag_ServerOnly() && (!IsValid(World) || !World->GetAuthGameMode());
	
	// Handle change of actor pooling counts. Clamp Maximum to at least 1 — recycle strategies index OutActors[0] / OutActors.Last() and would crash on an empty array if 0 were allowed through.
	Settings.MinimumActorCount = InNewSettings.MinimumActorCount;
	const int32 ClampedMaximumActorCount = FMath::Max(1, InNewSettings.MaximumActorCount);
	if (ClampedMaximumActorCount > Settings.MaximumActorCount)
	{
		InActors.Reserve(ClampedMaximumActorCount);
		OutActors.Reserve(ClampedMaximumActorCount);
	}
	Settings.MaximumActorCount = ClampedMaximumActorCount;

	// Update strategy
	Settings.Strategy = InNewSettings.Strategy;
	Settings.StorageTransform = InNewSettings.StorageTransform;
	Settings.SpawnedTransform = InNewSettings.SpawnedTransform;
	
	// Update based on if we should tick - test usually don't have access to the system to time-slice
	UNActorPoolSubsystem* System = IsValid(World) ? UNActorPoolSubsystem::Get(World) : nullptr;
	if (System != nullptr)
	{
		if (InNewSettings.CreateObjectsPerTick <= 0 && System->HasTickableActorPool(this))
		{
			System->RemoveTickableActorPool(this);
		}
		else if(InNewSettings.CreateObjectsPerTick > 0 && !System->HasTickableActorPool(this))
		{
			System->AddTickableActorPool(this);
		}
		
		Settings.CreateObjectsPerTick = InNewSettings.CreateObjectsPerTick;
	}
	else
	{
		// Because we have no system we need to create normally.
		Settings.CreateObjectsPerTick = -1;
	}
}

bool FNActorPool::ApplyStrategy()
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return false;
	
	switch (Settings.Strategy)
	{
		using enum ENActorPoolStrategy;
	case Create:
		return CreateActors();
	case CreateLimited:
		if (OutActors.Num() >= Settings.MaximumActorCount)
			return false;
		return CreateActors();
	case CreateRecycleFirst:
		if (OutActors.Num() >= Settings.MaximumActorCount)
			return ReturnAtIndex(0);
		return CreateActors();
	case CreateRecycleLast:
		if (OutActors.Num() >= Settings.MaximumActorCount)
			return ReturnAtIndex(OutActors.Num() - 1);
		return CreateActors();
	case Fixed:
		// Self-heal: warm-up for a Fixed pool is normally time-sliced by Tick() toward MinimumActorCount.
		// If that tick was never delivered (e.g. the pool was created in a world that won't tick again),
		// the pool would otherwise stay permanently empty since Fixed creates nothing on demand. Fill the
		// remaining deficit here. A warmed-then-exhausted pool has TotalActors == MinimumActorCount and
		// correctly falls through to return false.
		return WarmUpDeficit();
	case FixedRecycleFirst:
		// OutActors empty means nothing is checked out to recycle: either the pool is under-warmed (fill
		// the deficit, mirroring Fixed) or genuinely sized to zero, where there is nothing to hand back.
		if (OutActors.IsEmpty()) return WarmUpDeficit();
		return ReturnAtIndex(0);
	case FixedRecycleLast:
		if (OutActors.IsEmpty()) return WarmUpDeficit();
		return ReturnAtIndex(OutActors.Num() - 1);
	default:
		return false;
	}
}

bool FNActorPool::WarmUpDeficit()
{
	// Shared on-demand warm-up for the Fixed* strategies. Mirrors Tick()/Fill() deficit math, but creates the
	// whole remaining deficit in one shot because the caller (Get/Spawn) needs a poppable actor now rather than
	// across future ticks. Stub pools never reach here — ApplyStrategy short-circuits them before this call.
	const int32 Total = InActors.Num() + OutActors.Num();
	if (Total < Settings.MinimumActorCount)
	{
		ReserveForPoolSize(Settings.MinimumActorCount);
		return CreateActors(Settings.MinimumActorCount - Total);
	}
	return false;
}

void FNActorPool::ReserveForPoolSize(const int32 PoolSize)
{
	// Reserve only grows, so this is a no-op once both arrays already hold the target capacity.
	InActors.Reserve(PoolSize);
	OutActors.Reserve(PoolSize);
}

bool FNActorPool::CreateActors(const int32 Count)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return true;
	
	// We didnt make anything
	if (Count == 0) return false;
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = Settings.HasFlag_DeferConstruction();
	// We need to tell the spawn to occur and not warn about collisions.
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Array capacity is reserved by the warm-up callers (Fill/Prewarm/Tick) against their known target via
	// ReserveForPoolSize; the on-demand Count == 1 path relies on TArray's geometric growth.

#if WITH_EDITOR
	int32 LabelNumber = (InActors.Num() + OutActors.Num());
#endif // WITH_EDITOR

	// Create actual actors
	bool bCreatedActors = true;
	for (int32 i = 0; i < Count; i++)
	{
#if WITH_EDITOR
		const FString Label = FString::Printf(TEXT("%s__%i"), *this->Name, LabelNumber++);
		SpawnInfo.InitialActorLabel = Label;
#endif // WITH_EDITOR
		if (!CreateActor(SpawnInfo))
		{
			bCreatedActors = false;
		}
	}
	return bCreatedActors;
}

bool FNActorPool::CreateActor(const FActorSpawnParameters& SpawnInfo)
{
	AActor* CreatedActor = World->SpawnActorAbsolute(Template, Settings.StorageTransform, SpawnInfo);
	if (CreatedActor == nullptr)
	{
		UE_LOG(LogNexusActorPools, Error, TEXT("FNActorPool failed to spawn actor of class %s when requested/needed."), *Template->GetName());
		return false;
	}

	if (bImplementsPoolItemInterface)
	{
		INActorPoolItem* ActorItem = Cast<INActorPoolItem>(CreatedActor);
		ActorItem->InitializeActorPoolItem(this);

		if (SpawnInfo.bDeferConstruction)
		{
			ActorItem->OnDeferredConstruction();
			CreatedActor->FinishSpawning(Settings.StorageTransform);
		}
		ActorItem->OnCreatedByActorPool();
			
		ApplyReturnState(CreatedActor);
		ActorItem->OnReturnToActorPool();
	}
	else
	{
		bool bInvokeFunctions = Settings.HasFlag_InvokeUFunctions();
		
		if (SpawnInfo.bDeferConstruction && bInvokeFunctions)
		{
			UFunction* OnDeferredConstructionFunction = CreatedActor->FindFunction(NEXUS::ActorPools::InvokeMethods::OnDeferredConstruction);
			if (OnDeferredConstructionFunction)
			{
				CreatedActor->ProcessEvent(OnDeferredConstructionFunction, nullptr);
			}
		}
		
		if (SpawnInfo.bDeferConstruction)
		{
			CreatedActor->FinishSpawning(Settings.StorageTransform);
		}
		
		
			
		if (bInvokeFunctions) // SLOW PATH
		{
			UFunction* OnCreatedByActorPoolFunction = CreatedActor->FindFunction(NEXUS::ActorPools::InvokeMethods::OnCreatedByActorPool);
			if (OnCreatedByActorPoolFunction)
			{
				CreatedActor->ProcessEvent(OnCreatedByActorPoolFunction, nullptr);
			}
		}
			
		ApplyReturnState(CreatedActor);
	}

	InActors.Add(CreatedActor);
	return true;
}

void FNActorPool::ApplySpawnState(AActor* Actor, const FVector& InPosition, const FRotator& InRotation) const
{
	// Set the network flag first -- Thanks Nick!
	if (Settings.HasFlag_SetNetDormancy())
	{
		Actor->SetNetDormancy(DORM_Awake);
	}

	// Set Actor Location And Rotation
	const FTransform SpawnTransform(
		InRotation.Quaternion() * Settings.SpawnedTransform.GetRotation(), 
		InPosition + Settings.SpawnedTransform.GetLocation(),
		Settings.SpawnedTransform.GetScale3D());

	Actor->SetActorTransform(SpawnTransform,Settings.HasFlag_SweepBeforeSettingLocation(), nullptr, ETeleportType::ResetPhysics);
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(World, LogNexusActorPools, Verbose, InPosition, NEXUS::ActorPools::VLog::PointSize, NEXUS::ActorPools::VLog::RequestedPointColor, TEXT("%s"), *Actor->GetName());
	UE_VLOG_LOCATION(World, LogNexusActorPools, Verbose, Actor->GetActorLocation(), NEXUS::ActorPools::VLog::PointSize, NEXUS::ActorPools::VLog::ActualPointColor, TEXT("%s"), *Actor->GetName());
#endif // ENABLE_VISUAL_LOG
	
	Actor->SetActorTickEnabled(Actor->PrimaryActorTick.bStartWithTickEnabled);
	
	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		PrimitiveComponent->ResetSceneVelocity();
		if (SpawnPhysicsSimulation == ENToggle::Enabled)
		{
			PrimitiveComponent->SetSimulatePhysics(true);
		}
		else if (SpawnPhysicsSimulation == ENToggle::Disabled)
		{
			PrimitiveComponent->SetSimulatePhysics(false);
		}
		
		PrimitiveComponent->MarkRenderStateDirty();
	}
	
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
}

void FNActorPool::ApplyReturnState(AActor* Actor) const
{
	// Move to a storage location
	if (Settings.HasFlag_ReturnToStorage())
	{
		Actor->SetActorTransform(Settings.StorageTransform, false, nullptr, ETeleportType::ResetPhysics);
	}
	
	Actor->SetActorTickEnabled(false);
	Actor->SetActorHiddenInGame(true);

	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		PrimitiveComponent->ResetSceneVelocity();
		if (SpawnPhysicsSimulation == ENToggle::Enabled)
		{
			PrimitiveComponent->SetSimulatePhysics(false);
		}
		PrimitiveComponent->MarkRenderStateDirty();
	}
	Actor->SetActorEnableCollision(false);

	if (Settings.HasFlag_SetNetDormancy())
	{
		Actor->SetNetDormancy(DORM_Initial);
	}
}

void FNActorPool::Clear(const bool bForceDestroy)
{
	for (int32 i = InActors.Num() - 1; i >= 0; i--)
	{
		ReleaseActor(InActors[i], bForceDestroy);
	}
	for (int32 i = OutActors.Num() - 1; i >= 0; i--)
	{
		ReleaseActor(OutActors[i], bForceDestroy);
	}
	
	InActors.Reset();
	OutActors.Reset();
}

void FNActorPool::ReleaseActor(const TObjectPtr<AActor> Actor, const bool bForceDestroy) const
{
	// Always remove if it is rooted
	if (!IsValid(Actor) || Actor->IsPendingKillPending())
	{
		return;
	}

	const bool bBroadcastRelease = Settings.HasFlag_BroadcastRelease();
	if (bImplementsPoolItemInterface && bBroadcastRelease)
	{
		INActorPoolItem* ActorItem = Cast<INActorPoolItem>(Actor);
		ActorItem->OnReleasedFromActorPool();
	}
	else if (bBroadcastRelease && Settings.HasFlag_InvokeUFunctions()) // SLOW PATH
	{
		UFunction* Function = Actor->FindFunction(NEXUS::ActorPools::InvokeMethods::OnReleasedFromActorPool);
		if (Function)
		{
			Actor->ProcessEvent(Function, nullptr);
		}
	}
	
	if (bForceDestroy)
	{
		if (Actor->IsRooted())
		{
			UE_LOG(LogNexusActorPools, Warning, TEXT("An FNActorPool was told to release AND destroy an Actor(%s) which was rooted. Skipping the destroy, this is probably a mistake."), *Actor->GetName());
			return;
		}
		
		Actor->Destroy();
	}
}

void FNActorPool::Fill()
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;

	// MinimumActorCount is a prewarm target measured against the whole pool (in + out), matching Tick().
	// At the only call site (PostInitialize) OutActors is empty, so this is behaviourally identical today —
	// it just keeps the two warm-up paths reading the same if Fill() is ever called after actors are checked out.
	const int32 Deficit = Settings.MinimumActorCount - (InActors.Num() + OutActors.Num());
	if (Deficit > 0)
	{
		ReserveForPoolSize(Settings.MinimumActorCount);
		UE_LOG(LogNexusActorPools, Verbose, TEXT("Filling FNActorPool(%s) to %i items (+%i)"), *Template->GetName(), Settings.MinimumActorCount, Deficit);
		CreateActors(Deficit);
	}
}

void FNActorPool::Prewarm(const int32 Count)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;
	
	ReserveForPoolSize(InActors.Num() + OutActors.Num() + Count);
	UE_LOG(LogNexusActorPools, Verbose, TEXT("Warming FNActorPool(%s) with %i items."), *Template->GetName(), Count);
	CreateActors(Count);
}

bool FNActorPool::Tick()
{
	// A stub pool has nothing to warm — let the caller drop it from the tick list.
	if (bStubMode) return false;

	if (const int32 TotalActors = InActors.Num() + OutActors.Num();
		TotalActors < Settings.MinimumActorCount)
	{
		// Reserve to the final warm-up target once; Reserve is grow-only, so later ticks don't re-allocate.
		ReserveForPoolSize(Settings.MinimumActorCount);
		CreateActors(FMath::Min(Settings.CreateObjectsPerTick, (Settings.MinimumActorCount - TotalActors)));
		return true; // still warming
	}

	// Reached the minimum — warm-up is a one-shot, so we no longer need to tick.
	return false;
}
