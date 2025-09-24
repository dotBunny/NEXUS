// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPool.h"
#include "INActorPoolItem.h"
#include "NActorPoolsSettings.h"
#include "NActorPoolSubsystem.h"
#include "NActorUtils.h"
#include "NCoreMinimal.h"

#if WITH_EDITOR
int32 FNActorPool::ActorPoolTicket = 0;
#endif

FNActorPool::FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass)
{
	PreInitialize(TargetWorld, ActorClass);
	if (bImplementsInterface)
	{
		AActor* DefaultActor = ActorClass->GetDefaultObject<AActor>();
		UpdateSettings((Cast<INActorPoolItem>(DefaultActor))->GetActorPoolSettings());
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

void FNActorPool::PreInitialize(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass)
{
	World = TargetWorld;
	Template = ActorClass;
	USceneComponent* RootComponent = FNActorUtils::GetRootComponentFromDefaultObject(ActorClass);
	bImplementsInterface = Template->ImplementsInterface(UNActorPoolItem::StaticClass());

	// The ActorPool system requires that the root component of its actors be used to determine the physics settings.
	if (RootComponent != nullptr)
	{
		if (const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(RootComponent))
		{
			if (PrimitiveComponent->BodyInstance.bSimulatePhysics)
			{
				SpawnPhysicsSimulation = T_True;
			}
			else
			{
				SpawnPhysicsSimulation = T_False;
			}
		}
	}

#if WITH_EDITOR
	ActorPoolTicket++;

	// Generate the name for the actor pool
	FString WorkingName = Template->GetName();
	if (WorkingName.EndsWith(TEXT("_C")))
	{
		WorkingName = WorkingName.Mid(0, WorkingName.Len() - 2);
	}
	Name = FString::Printf(TEXT("AP_%s_%i"), *WorkingName, ActorPoolTicket);
#endif
}

void FNActorPool::PostInitialize()
{
	// Cache half-height of the item	
	if (const APawn* Pawn = Cast<APawn>(Template->GetDefaultObject()))
	{
		HalfHeight = Pawn->GetDefaultHalfHeight();
		HalfHeightOffset = FVector(0.0f, 0.0f, HalfHeight);
		bHasHalfHeight = true;
	}
	else
	{
		bHasHalfHeight = false;
	}

	if (Settings.CreateObjectsPerTick == -1)
	{
		Fill();
	}
}

AActor* FNActorPool::Get()
{
	if (InActors.Num() == 0 && !ApplyStrategy())
	{
		return nullptr;
	}

	AActor* ReturnActor = InActors.Pop();
	OutActors.Add(ReturnActor);
	return ReturnActor;
}

AActor* FNActorPool::Spawn(const FVector& Position, const FRotator& Rotation)
{
	if (InActors.Num() == 0 && !ApplyStrategy())
	{
		return nullptr;
	}

	AActor* ReturnActor = InActors.Pop();
	OutActors.Add(ReturnActor); // TODO: Maybe we could optimize the out actors array to match the actual counts?
	ApplySpawnState(ReturnActor, Position, Rotation);
	
	if (bImplementsInterface)
	{
		(Cast<INActorPoolItem>(ReturnActor))->OnSpawnedFromActorPool();
	}

	return ReturnActor;
}

bool FNActorPool::Return(AActor* Actor)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return true;
	
	if (Actor == nullptr)
	{
		N_LOG(Warning, TEXT("[FNActorPool::Return] Attempting to return a null actor."));
		return false;
	}
	
	ApplyReturnState(Actor);

	// We have to manage the position a bit based on the strategy.
	switch (Settings.Strategy)
	{
	case APS_CreateRecycleFirst:
	case APS_CreateRecycleLast:
	case APS_FixedRecycleFirst:
	case APS_FixedRecycleLast:
		OutActors.Remove(Actor);
		break;
	default:
		OutActors.RemoveSwap(Actor, EAllowShrinking::No);
		break;
	}
	
	InActors.Add(Actor);

	if (bImplementsInterface)
	{
		(Cast<INActorPoolItem>(Actor))->OnReturnToActorPool();
	}

	return true;
}

void FNActorPool::UpdateSettings(const FNActorPoolSettings& InNewSettings)
{
	// Ingest flags - and update cached flags
	Settings.Flags = InNewSettings.Flags;
	bStubMode = Settings.HasFlag_ServerOnly() && !World->GetAuthGameMode();
	
	// Handle change of actor pooling counts
	Settings.MinimumActorCount = InNewSettings.MinimumActorCount;
	if (InNewSettings.MaximumActorCount > Settings.MaximumActorCount)
	{
		InActors.Reserve(InNewSettings.MaximumActorCount);
		OutActors.Reserve(InNewSettings.MaximumActorCount);
	}
	Settings.MaximumActorCount = InNewSettings.MaximumActorCount;

	// Update strategy
	Settings.Strategy = InNewSettings.Strategy;
	Settings.DefaultTransform = InNewSettings.DefaultTransform;
	
	// Update based on if we should tick - test usually don't have access to the system to time-slice
	UNActorPoolSubsystem* System  = UNActorPoolSubsystem::Get(World);
	if (System != nullptr)
	{
		if (InNewSettings.CreateObjectsPerTick <= 0 && System->HasTickableActorPool(this))
		{
			UNActorPoolSubsystem::Get(World)->RemoveTickableActorPool(this);
		}
		else if(InNewSettings.CreateObjectsPerTick > 0 && !System->HasTickableActorPool(this))
		{
			UNActorPoolSubsystem::Get(World)->AddTickableActorPool(this);
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
	case APS_Create:
		CreateActor();
		return true;
	case APS_CreateLimited:
		if (OutActors.Num() >= Settings.MaximumActorCount)
			return false;
		CreateActor();
		return true;
	case APS_CreateRecycleFirst:
		if (OutActors.Num() >= Settings.MaximumActorCount)
		{
			Return(OutActors[0]);
		}
		else
		{
			CreateActor();
		}
		return true;
	case APS_CreateRecycleLast:
		if (OutActors.Num() >= Settings.MaximumActorCount)
		{
			Return(OutActors[OutActors.Num() - 1]);
		}
		else
		{
			CreateActor();
		}
		return true;
	case APS_Fixed:
		return false;
	case APS_FixedRecycleFirst:
		Return(OutActors[0]);
		return true;
	case APS_FixedRecycleLast:
		Return(OutActors[OutActors.Num() - 1]);
		return true;
	default:
		return false;
	}
}

void FNActorPool::CreateActor(const int32 Count)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient & RF_MarkAsRootSet; // AddToRoot - Pool will manage lifecycle
	SpawnInfo.bDeferConstruction = Settings.HasFlag_DeferConstruction();
	// We need to tell the spawn to occur and not warn about collisions.
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Reserve space for the new actors
	if (Count > 1)
	{
		InActors.Reserve(InActors.Num() + Count);
		OutActors.Reserve(OutActors.Num() + Count);
	}
	
#if WITH_EDITOR
	int32 LabelNumber = (InActors.Num() + OutActors.Num());
#endif

	// Create actual actors
	for (int i = 0; i < Count; i++)
	{
#if WITH_EDITOR
		const FString Label = FString::Printf(TEXT("%s__%i"), *this->Name, LabelNumber++);
		SpawnInfo.InitialActorLabel = Label;
#endif

		AActor* CreatedActor = World->SpawnActorAbsolute(Template, Settings.DefaultTransform, SpawnInfo);

		if (bImplementsInterface)
		{
			INActorPoolItem* ActorItem = Cast<INActorPoolItem>(CreatedActor);
			ActorItem->InitializeActorPoolItem(this);

			if (SpawnInfo.bDeferConstruction)
			{
				ActorItem->OnDeferredConstruction();
				CreatedActor->FinishSpawning(Settings.DefaultTransform);
			}
			ActorItem->OnCreatedByActorPool();
			ApplyReturnState(CreatedActor);
			ActorItem->OnReturnToActorPool();
		}
		else
		{
			if (SpawnInfo.bDeferConstruction && Settings.HasFlag_ShouldFinishSpawning())
			{
				CreatedActor->FinishSpawning(Settings.DefaultTransform);
			}
			ApplyReturnState(CreatedActor);
		}
	
		InActors.Add(CreatedActor);
	}
}

void FNActorPool::ApplySpawnState(AActor* Actor, const FVector& InPosition, const FRotator& InRotation) const
{
	// Set the network flag first -- Thanks Nick!
	if (Settings.HasFlag_SetNetDormancy())
	{
		Actor->SetNetDormancy(DORM_Awake);
	}

	// Set Actor Location And Rotation
	if (bHasHalfHeight)
	{
		Actor->SetActorTransform(
			FTransform(InRotation, InPosition + HalfHeightOffset, Settings.DefaultTransform.GetScale3D()),
			Settings.HasFlag_SweepBeforeSettingLocation(), nullptr, ETeleportType::ResetPhysics);
	}
	else
	{
		Actor->SetActorTransform(
			FTransform(InRotation, InPosition, Settings.DefaultTransform.GetScale3D()),
			Settings.HasFlag_SweepBeforeSettingLocation(), nullptr, ETeleportType::ResetPhysics);
	}
	
	Actor->SetActorTickEnabled(Actor->PrimaryActorTick.bStartWithTickEnabled);
	
	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		PrimitiveComponent->ResetSceneVelocity();
		if (SpawnPhysicsSimulation == T_True)
		{
			PrimitiveComponent->SetSimulatePhysics(true);
		}
		else if (SpawnPhysicsSimulation == T_False)
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
	if (Settings.HasFlag_ReturnToStorageLocation())
	{
		Actor->SetActorLocation(Settings.DefaultTransform.GetLocation(), false, nullptr, ETeleportType::ResetPhysics);
	}

	Actor->SetActorTickEnabled(false);
	Actor->SetActorHiddenInGame(true);

	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		PrimitiveComponent->ResetSceneVelocity();
		if (SpawnPhysicsSimulation == T_True)
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
		check(InActors[i] != nullptr);
		if (InActors[i]->IsPendingKillPending())
		{
			continue;
		}
		InActors[i]->RemoveFromRoot();
		if (bImplementsInterface && Settings.HasFlag_BroadcastDestroy())
		{
			INActorPoolItem* ActorItem = Cast<INActorPoolItem>(InActors[i]);
			ActorItem->OnDestroyedByActorPool();
		}
		if (bForceDestroy)
		{
			InActors[i]->Destroy();
		}
	}
	for (int32 i = OutActors.Num() - 1; i >= 0; i--)
	{
		check(OutActors[i] != nullptr);

		if (OutActors[i]->IsPendingKillPending())
		{
			continue;
		}
		OutActors[i]->RemoveFromRoot();
		if (bImplementsInterface && Settings.HasFlag_BroadcastDestroy())
		{
			INActorPoolItem* ActorItem = Cast<INActorPoolItem>(OutActors[i]);
			ActorItem->OnDestroyedByActorPool();
		}
		
		if (bForceDestroy)
		{
			OutActors[i]->Destroy();
		}
	}
	InActors.Reset();
	OutActors.Reset();
}

void FNActorPool::Fill()
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;
	
	N_LOG(Log, TEXT("[FNActorPool::Fill] Filling pool %s to %i items."), *Template->GetName(), Settings.MinimumActorCount)
	CreateActor(Settings.MinimumActorCount - InActors.Num());
}

void FNActorPool::Prewarm(const int32 Count)
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;
	
	N_LOG(Log, TEXT("[FNActorPool::Prewarm] Warming pool %s with %i items."), *Template->GetName(), Count)
	CreateActor(Count);
}

void FNActorPool::Tick()
{
	// Ensure the pool is a stub when WorldAuthority is flagged.
	if (bStubMode) return;
	
	if (const int32 TotalActors = InActors.Num() + OutActors.Num();
		TotalActors < Settings.MinimumActorCount)
	{
		CreateActor(FMath::Min(Settings.CreateObjectsPerTick, (Settings.MinimumActorCount - TotalActors)));
	}
}
