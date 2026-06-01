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

#if WITH_EDITOR
#include "Editor.h"
#include "Selection.h"
#endif // WITH_EDITOR

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

ANCellProxy* ANCellProxy::CreateInstance(UWorld* World, FNAssemblyGraphCellNode* CellNode, const FNCellAssemblyData& InstanceData, const bool bPreLoadLevel)
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
	
	Proxy->AssemblyData = InstanceData;
	
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
	
	// Moving outside this we cant be sure we will have a CellNode again.
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
	
	// Replicated assembly data
	LevelInstance->AssemblyData = AssemblyData;
	
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

void ANCellProxy::UnloadLevelInstance(const bool bTagActorsToIgnore) const
{
	if (LevelInstance != nullptr)
	{
		UWorld* World = GetWorld();
		ULevelInstanceSubsystem* Subsystem = World->GetSubsystem<ULevelInstanceSubsystem>();
		
		// We are going to iterate the Actors and flag them to be ignored by the next generation
		// We are going to iterate the Actors and flag them to be ignored by the next generation
		if (bTagActorsToIgnore)
		{
			TagActorsToIgnore();
		}
		
#if WITH_EDITOR
		// Inflight loading currently
		if (Subsystem->IsLoading(LevelInstance))
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
	}
	Show();
}

void ANCellProxy::DestroyLevelInstance(const bool bUnregisterCellLevelInstance, const bool bTagActorsToIgnore)
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

		// We are going to iterate the Actors and flag them to be ignored by the next generation
		if (bTagActorsToIgnore)
		{
			TagActorsToIgnore();
		}

#if WITH_EDITOR
		// Drop any selection handles into this level instance / its components before destruction so
		// the editor's typed-element registry does not assert on a stale handle next mouse-move.
		if (GIsEditor && GEditor != nullptr)
		{
			if (USelection* ActorSelection = GEditor->GetSelectedActors())
			{
				ActorSelection->Deselect(LevelInstance);
			}
			if (USelection* ComponentSelection = GEditor->GetSelectedComponents())
			{
				for (UActorComponent* Component : LevelInstance->GetComponents())
				{
					ComponentSelection->Deselect(Component);
				}
			}
		}

		// Inflight loading currently
		if (Subsystem->IsLoading(LevelInstance))
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
	}
	
	// If we're tagging the actors that means were making something new right away so lets not show the mesh
	if (!bTagActorsToIgnore)
	{
		Show();
	}
}

void ANCellProxy::InitializeFromNCell(UNCell* InCell)
{
	if (InCell == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("Attempted to initialize an ANCellProxy(%s) with a null UNCell."), *this->GetName());
		return;
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

void ANCellProxy::TagActorsToIgnore() const
{
	if (LevelInstance == nullptr) return;
	
	ULevel* LoadedLevel = LevelInstance->GetLoadedLevel();
	if (LoadedLevel == nullptr) return;
	
	// We are going to iterate the Actors and flag them to be ignored by the next generation
	for (AActor* Actor : LoadedLevel->Actors)
	{
		if (!IsValid(Actor)) continue;
		Actor->Tags.Add(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
	}
}
