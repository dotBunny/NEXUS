// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellProxy.h"

#include "NWorldAssemblyRegistry.h"
#include "Cell/NCell.h"
#include "Cell/NCellLevelInstance.h"
#include "NWorldAssemblySettings.h"
#include "Components/BillboardComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "LevelInstance/LevelInstanceActor.h"

ANCellProxy::ANCellProxy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// TODO: Is the proxy sync option doable?
	// if (UNWorldAssemblySettings::Get()->NetworkingMode == ENWorldAssemblyNetworkMode::AlwaysRelevantCellProxies)
	// {
	// 	bReplicates = true;
	// 	bAlwaysRelevant = true;
	// }
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// Add our physics shape
	Mesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->Mobility = EComponentMobility::Movable;
	
	// Mesh Visible-stuff
	Mesh->SetVisibleInRayTracing(false);
	Mesh->bExplicitShowWireframe = true;
	Mesh->WireframeColor = FLinearColor::Gray;
	Mesh->SetTangentsType(EDynamicMeshComponentTangentsMode::AutoCalculated);
	
#if WITH_EDITOR	
	bCanPlayFromHere = 0;
#endif // WITH_EDITOR
	
	N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT("/NexusWorldAssembly/EditorResources/S_NCellProxy", RootComponent, false, 0.5f)
}

ANCellProxy* ANCellProxy::CreateInstance(UWorld* World, const uint32& OperationTicket, const FNAssemblyGraphCellNode* CellNode, const bool bPreLoadLevel)
{

	FActorSpawnParameters SpawnInfo;
	
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ANCellProxy* Proxy = World->SpawnActor<ANCellProxy>(CellNode->GetWorldPosition(), CellNode->GetWorldRotation(), SpawnInfo);
	
	if (Proxy == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("ANCellProxy was unable to CreateInstance, returning a nullptr; most likely caused by the World in a bad state."));
		return nullptr;
	}
	
	// Straight application of settings
	Proxy->OperationTicket = OperationTicket;
	
	// Context initialization
	Proxy->InitializeFromCellNode(CellNode);
	
	// Default initialization
	Proxy->InitializeFromNCell(CellNode->GetTemplate());
	
	// TODO: Do something with this || this only works on server, clients wont get preload, is it even worth it?
	if (bPreLoadLevel)
	{
		
		CellNode->GetTemplate()->World.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
								[](const FSoftObjectPath&, UObject* InLoadedObject)
								{
								}));	
	}
	return Proxy;
}

void ANCellProxy::CreateLevelInstance()
{
	if (LevelInstance != nullptr)
	{
		return;
	}
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this; 
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bDeferConstruction = true;
	
	// Spawn Actor
	const FVector SpawnLocation = this->GetActorLocation();
	const FRotator SpawnRotation = this->GetActorRotation();
	
	AActor* SpawnedLevelInstanceActor = GetWorld()->SpawnActor(
		ANCellLevelInstance::StaticClass(), &SpawnLocation, &SpawnRotation, SpawnInfo);
	
	// Cache reference to spawned actor
	LevelInstance = Cast<ANCellLevelInstance>(SpawnedLevelInstanceActor);
	
	// Replicated settings
	LevelInstance->OperationTicket = OperationTicket;
	JunctionsData.GenerateValueArray(LevelInstance->JunctionDetails);
	LevelInstance->FillJunctionData(); // this is really for the server
	
	// Apply relevancy flag to created actor
	if (bAlwaysRelevant)
	{
		LevelInstance->bAlwaysRelevant = true;
	}
	
	// Setup actor with our details
#if WITH_EDITOR
	LevelInstance->SetWorldAsset(Cell->World);
	LevelInstance->SetActorLabel(FString::Printf(TEXT("%s_LevelInstance"), *this->GetActorLabel()), false);
#endif // WITH_EDITOR
	LevelInstance->CookedWorldAsset = Cell->World;
	
	// Finalize the spawn, BeginPlay can now be called
	SpawnedLevelInstanceActor->FinishSpawning(this->GetActorTransform());
	
	// Register the CellLevelInstance
	FNWorldAssemblyRegistry::RegisterCellLevelInstance(LevelInstance);
}

void ANCellProxy::LoadLevelInstance(const bool bBlocking)
{
	if (LevelInstance == nullptr)
	{
		CreateLevelInstance();
	}
	
	ULevelInstanceSubsystem* Subsystem = GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();
		
	// Inflight loading currently
	if (!Subsystem->IsLoading(LevelInstance))
	{
#if WITH_EDITOR		
		if (bBlocking)
		{
			Subsystem->BlockLoadLevelInstance(LevelInstance);
		}
		else
		{
			LevelInstance->LoadLevelInstance();
		}
#else
		LevelInstance->LoadLevelInstance();
#endif		
	}
	
	Hide();
}

void ANCellProxy::UnloadLevelInstance(const bool bBlocking) const
{
	if (LevelInstance != nullptr)
	{
		UWorld* World = GetWorld();
		ULevelInstanceSubsystem* Subsystem = World->GetSubsystem<ULevelInstanceSubsystem>();
		
		// Snapshot the streamed sub-level before unload; the LI clears it during teardown and editor
		// unload can leave the level in World->Levels even after the LI actor is destroyed.
		ULevel* LoadedLevel = LevelInstance->GetLoadedLevel();
#if WITH_EDITOR
		// Inflight loading currently
		if (Subsystem->IsLoading(LevelInstance) || bBlocking)
		{
			Subsystem->BlockUnloadLevelInstance(LevelInstance);
		}
		else if (Subsystem->IsLoaded(LevelInstance))
		{
			Subsystem->RequestUnloadLevelInstance(LevelInstance);
		}
#else
		Subsystem->RequestUnloadLevelInstance(LevelInstance);
#endif // WITH_EDITOR
		
		if (LoadedLevel != nullptr)
		{
			World->RemoveLevel(LoadedLevel);
			// Maybe? World->FlushLevelStreaming();
			UPackage* Package = LoadedLevel->GetPackage();
			UWorld* PackageWorld = UWorld::FindWorldInPackage(Package);
			PackageWorld->DestroyWorld(false);
		}
	}
	Show();
}

void ANCellProxy::DestroyLevelInstance(bool bUnregisterCellLevelInstance, bool bBlocking)
{
	if (LevelInstance != nullptr)
	{
		UWorld* World = GetWorld();
		ULevelInstanceSubsystem* Subsystem = World->GetSubsystem<ULevelInstanceSubsystem>();

		// Unregister before the destruction starts
		if (bUnregisterCellLevelInstance)
		{
			FNWorldAssemblyRegistry::UnregisterCellLevelInstance(LevelInstance);
		}
		
		// Snapshot the streamed sub-level before unload; the LI clears it during teardown and editor
		// unload can leave the level in World->Levels even after the LI actor is destroyed.
		ULevel* LoadedLevel = LevelInstance->GetLoadedLevel();
		
#if WITH_EDITOR
		// Inflight loading currently
		if (Subsystem->IsLoading(LevelInstance) || bBlocking)
		{
			Subsystem->BlockUnloadLevelInstance(LevelInstance);
		}
		else if (Subsystem->IsLoaded(LevelInstance))
		{
			Subsystem->RequestUnloadLevelInstance(LevelInstance);
		}
#else
		Subsystem->RequestUnloadLevelInstance(LevelInstance);
#endif // WITH_EDITOR	


		LevelInstance->Destroy(true, false);
		LevelInstance = nullptr;
		if (LoadedLevel != nullptr)
		{
			World->RemoveLevel(LoadedLevel);
			// Maybe? World->FlushLevelStreaming();
			UPackage* Package = LoadedLevel->GetPackage();
			UWorld* PackageWorld = UWorld::FindWorldInPackage(Package);
			PackageWorld->DestroyWorld(false);
		}
	}
	Show();
}

void ANCellProxy::InitializeFromNCell(UNCell* InCell)
{
	if (InCell == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("Attempted to initialize an ANCellProxy with a null UNCell."));
	}
	
	Cell = InCell;

	// Prep dynamic mesh for changes
	Mesh->Modify();
	
	// Clear anything previous
#if WITH_EDITOR	
	Mesh->ClearAllCachedCookedPlatformData();
#endif // WITH_EDITOR

	// Convert our mesh data to UE
	Mesh->SetMesh(Cell->Root.Hull.CreateDynamicMesh(true));
	
	// Clear all collision
	SetActorEnableCollision(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Let's rock some colors
	Mesh->WireframeColor = Cell->Root.ProxyColor;
	
	// Create the material on placement (CDO settings access = bad)
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(UNWorldAssemblySettings::Get()->ProxyMaterial.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ANCellProxy::OnProxyMaterialLoaded));
}

void ANCellProxy::InitializeFromCellNode(const FNAssemblyGraphCellNode* CellNode)
{
	JunctionsData = CellNode->GetJunctions();
}

void ANCellProxy::OnProxyMaterialLoaded()
{
	if (!IsValid(Cell) || !IsValid(Mesh))
	{
		return;
	}

	UMaterialInterface* ProxyMaterial = UNWorldAssemblySettings::Get()->ProxyMaterial.Get();
	if (ProxyMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial = UMaterialInstanceDynamic::Create(ProxyMaterial, this);
	Mesh->SetMaterial(0, DynamicMaterial);

	DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), Cell->Root.ProxyColor);
	Mesh->MarkRenderStateDirty();
}

void ANCellProxy::Show() const
{
	Mesh->SetVisibility(true);
	Mesh->MarkRenderStateDirty();
}

void ANCellProxy::Hide() const
{
	Mesh->SetVisibility(false);
	Mesh->MarkRenderStateDirty();
}
