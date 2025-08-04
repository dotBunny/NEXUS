// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPool.h"
#include "INActorPoolItem.h"
#include "NActorPoolsSettings.h"
#include "NActorPoolSubsystem.h"
#include "NActorUtils.h"
#include "NCoreMinimal.h"

#if WITH_EDITOR
int FNActorPool::ActorPoolTicket = 0;
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
		TemplateScale = RootComponent->GetRelativeScale3D();
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
	// Cache half height of the item	
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
	// Handle change of actor pooling counts
	Settings.MinimumActorCount = InNewSettings.MinimumActorCount;
	Settings.MaximumActorCount = InNewSettings.MaximumActorCount;
	Settings.bAllowCreateMoreObjects = InNewSettings.bAllowCreateMoreObjects;
	if (Settings.MaximumActorCount == -1)
	{
		if (Settings.bAllowCreateMoreObjects)
		{
			const int NewSize = Settings.MinimumActorCount * 2;
			InActors.Reserve(NewSize);
			OutActors.Reserve(NewSize);
		}
		else
		{
			InActors.Reserve(Settings.MinimumActorCount);
			OutActors.Reserve(Settings.MinimumActorCount);
		}
	}
	else
	{
		InActors.Reserve(Settings.MaximumActorCount);
		OutActors.Reserve(Settings.MaximumActorCount);
	}

	// Update strategy
	Settings.Strategy = InNewSettings.Strategy;

	// Update based on if we should tick
	if (Settings.CreateObjectsPerTick && !InNewSettings.CreateObjectsPerTick)
	{
		UNActorPoolSubsystem::Get(World)->RemoveTickableActorPool(this);
	}
	else if (!Settings.CreateObjectsPerTick && InNewSettings.CreateObjectsPerTick)
	{
		UNActorPoolSubsystem::Get(World)->AddTickableActorPool(this);
	}
	Settings.CreateObjectsPerTick = InNewSettings.CreateObjectsPerTick;
	Settings.bSpawnSweepBeforeSettingLocation = InNewSettings.bSpawnSweepBeforeSettingLocation;
	Settings.bReturnMoveToLocation = InNewSettings.bReturnMoveToLocation;

	Settings.ReturnMoveLocation = InNewSettings.ReturnMoveLocation;
}

bool FNActorPool::ApplyStrategy()
{
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

void FNActorPool::CreateActor()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = Settings.bDeferConstruction;

	// We need to tell the spawn to occur and not warn about collisions.
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Create actual actor
	AActor* CreatedActor = World->SpawnActorAbsolute(Template, DefaultTransform, SpawnInfo);

	// Ensure the actor is not garbage collected
	CreatedActor->AddToRoot();

#if WITH_EDITOR
	CreatedActor->SetActorLabel(FString::Printf(TEXT("%s__%i"), *this->Name, (InActors.Num() + OutActors.Num())));
#endif
	
	if (bImplementsInterface)
	{
		INActorPoolItem* ActorItem = Cast<INActorPoolItem>(CreatedActor);
		ActorItem->InitializeActorPoolItem(this);

		if (Settings.bDeferConstruction)
		{
			ActorItem->OnDeferredConstruction();
			CreatedActor->FinishSpawning(DefaultTransform);
		}
		ActorItem->OnCreatedByActorPool();
		ApplyReturnState(CreatedActor);
		ActorItem->OnReturnToActorPool();
	}
	else
	{
		if (Settings.bDeferConstruction && Settings.bShouldFinishSpawning)
		{
			CreatedActor->FinishSpawning(DefaultTransform);
		}
		ApplyReturnState(CreatedActor);
	}
	
	InActors.Add(CreatedActor);
}


void FNActorPool::ApplySpawnState(AActor* Actor, const FVector& InPosition, const FRotator& InRotation) const
{
	// Set network flag first -- Thanks Nick!	
	Actor->SetNetDormancy(DORM_Awake);

	// Set Actor Location And Rotation
	if (bHasHalfHeight)
	{
		Actor->SetActorTransform(
			FTransform(InRotation, InPosition + HalfHeightOffset, TemplateScale),
			Settings.bSpawnSweepBeforeSettingLocation, nullptr, ETeleportType::ResetPhysics);
	}
	else
	{
		Actor->SetActorTransform(
			FTransform(InRotation, InPosition, TemplateScale),
			Settings.bSpawnSweepBeforeSettingLocation, nullptr, ETeleportType::ResetPhysics);
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
	// Move to storage location
	if (Settings.bReturnMoveToLocation)
	{
		Actor->SetActorLocation(Settings.ReturnMoveLocation, false, nullptr, ETeleportType::ResetPhysics);
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
	Actor->SetNetDormancy(DORM_Initial);
}

void FNActorPool::Clear(const bool bForceDestroy)
{
	for (int i = InActors.Num() - 1; i >= 0; i--)
	{
		check(InActors[i] != nullptr);
		if (InActors[i]->IsPendingKillPending())
		{
			continue;
		}
		InActors[i]->RemoveFromRoot();
		if (bForceDestroy)
		{
			InActors[i]->Destroy();
		}
	}
	for (int i = OutActors.Num() - 1; i >= 0; i--)
	{
		check(OutActors[i] != nullptr);

		if (OutActors[i]->IsPendingKillPending())
		{
			continue;
		}
		OutActors[i]->RemoveFromRoot();
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
	N_LOG(Log, TEXT("[FNActorPool::Fill] Filling pool %s to %i items."), *Template->GetName(), Settings.MinimumActorCount)
	for (int i = InActors.Num(); i < Settings.MinimumActorCount; i++)
	{
		CreateActor();
	}
}

void FNActorPool::Warm(const int Count)
{
	N_LOG(Log, TEXT("[FNActorPool::Warm] Warming pool %s with %i items."), *Template->GetName(), Count)
	for (int i = 0; i < Count; i++)
	{
		CreateActor();
	}
}

void FNActorPool::Tick()
{
	if (const int TotalActors = InActors.Num() + OutActors.Num();
		TotalActors < Settings.MinimumActorCount)
	{
		const int SpawnCountThisTick = FMath::Min((Settings.MinimumActorCount - TotalActors), Settings.CreateObjectsPerTick);
		for (int i = 0; i < SpawnCountThisTick; i++)
		{
			CreateActor();
		}
	}
}
