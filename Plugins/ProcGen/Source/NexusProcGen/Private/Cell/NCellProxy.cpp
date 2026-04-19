// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellProxy.h"

#include "Cell/NCell.h"
#include "Cell/NCellLevelInstance.h"
#include "NProcGenSettings.h"
#include "Components/BillboardComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "LevelInstance/LevelInstanceActor.h"

ANCellProxy::ANCellProxy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// TODO: Is the proxy sync option doable?
	// if (UNProcGenSettings::Get()->NetworkingMode == ENProcGenNetworkMode::AlwaysRelevantCellProxies)
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
	Mesh->SetEnableRaytracing(false);
	Mesh->SetVisibleInRayTracing(false);
	Mesh->bExplicitShowWireframe = true;
	Mesh->WireframeColor = FLinearColor::Gray;
	Mesh->SetTangentsType(EDynamicMeshComponentTangentsMode::AutoCalculated);
	
	// Mesh Collisions
	//Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//Mesh->SetCollisionObjectType(ECC_WorldStatic);
	//Mesh->EnableComplexAsSimpleCollision();
	
#if WITH_EDITOR	
	bCanPlayFromHere = 0;
#endif // WITH_EDITOR
	
	N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT("/NexusProcGen/EditorResources/S_NCellProxy", RootComponent, false, 0.5f)
}

ANCellProxy* ANCellProxy::CreateInstance(UWorld* World, const FNProcGenGraphCellNode* CellNode, const bool bPreLoadLevel)
{

	FActorSpawnParameters SpawnInfo;
	
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ANCellProxy* Proxy = World->SpawnActor<ANCellProxy>(CellNode->GetWorldPosition(), CellNode->GetWorldRotation(), SpawnInfo);
	
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
	
	// Spawn Actor
	const FVector SpawnLocation = this->GetActorLocation();
	const FRotator SpawnRotation = this->GetActorRotation();
	AActor* SpawnedLevelInstanceActor = GetWorld()->SpawnActor(
		ANCellLevelInstance::StaticClass(), &SpawnLocation, &SpawnRotation, SpawnInfo);
	
	// Cache reference to spawned actor
	LevelInstance = Cast<ANCellLevelInstance>(SpawnedLevelInstanceActor);
	
	// Apply relevancy flag to created actor
	if (bAlwaysRelevant)
	{
		LevelInstance->bAlwaysRelevant = true;
	}
	
	// Setup actor with our details
#if WITH_EDITOR
	LevelInstance->SetWorldAsset(NCell->World);
	LevelInstance->SetActorLabel(FString::Printf(TEXT("%s_LevelInstance"), *this->GetActorLabel()), false);
#endif // WITH_EDITOR
	LevelInstance->SetCookedWorldAsset(NCell->World);
	
	// Add data for junction (not synced!!)

	LevelInstance->JunctionData = &JunctionsData;
}

void ANCellProxy::LoadLevelInstance()
{
	if (LevelInstance == nullptr)
	{
		CreateLevelInstance();
	}
	
	// Something about IsLoaded occasionally causes a crash?
	if (!LevelInstance->IsLoaded())
	{
		LevelInstance->LoadLevelInstance();

		// TODO: Hook up Actor? / Root Component / Flag as being sapwned?
		
		Hide();
	}
}

void ANCellProxy::UnloadLevelInstance() const
{
	if (LevelInstance != nullptr && LevelInstance->IsLoaded())
	{
		LevelInstance->UnloadLevelInstance();
	}
	Show();
}

void ANCellProxy::DestroyLevelInstance()
{
	if (LevelInstance != nullptr)
	{
		if (LevelInstance->IsLoaded())
		{
			LevelInstance->UnloadLevelInstance();
		}
		LevelInstance->Destroy(true, false);
		LevelInstance = nullptr;
	}
	Show();
}

void ANCellProxy::InitializeFromNCell(UNCell* InNCell)
{
	NCell = InNCell;

	// Prep dynamic mesh for changes
	Mesh->Modify();
	
	// Clear anything previous
#if WITH_EDITOR	
	Mesh->ClearAllCachedCookedPlatformData();
#endif // WITH_EDITOR

	// Convert our mesh data to UE
	Mesh->SetMesh(NCell->Root.Hull.CreateDynamicMesh(true));

	
	// DISABLE IT FOR NOW? WHY DO WE NEED IT?
	SetActorEnableCollision(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// // Setup collision
	// UBodySetup* BodySetup = Mesh->GetBodySetup();
	// if (BodySetup != nullptr)
	// {
	// 	BodySetup->Modify();
	// }
	//
	// FKConvexElem ConvexHull;
	// ConvexHull.VertexData = NCell->Root.Hull.Vertices;
	// ConvexHull.IndexData = NCell->Root.Hull.GetFlatIndices();
	// ConvexHull.CalcAABB(FTransform::Identity, FVector::One());
	//
	// FKAggregateGeom AggGeom;;
	// AggGeom.ConvexElems.Add(ConvexHull);
	//
	// Mesh->SetSimpleCollisionShapes(AggGeom, true);		

	// Let's rock some colors
	Mesh->WireframeColor = NCell->Root.ProxyColor;
	
	// Create the material on placement (CDO settings access = bad)
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(UNProcGenSettings::Get()->ProxyMaterial.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ANCellProxy::OnProxyMaterialLoaded));
}

void ANCellProxy::InitializeFromCellNode(const FNProcGenGraphCellNode* CellNode)
{
	JunctionsData = CellNode->GetJunctions();
}

void ANCellProxy::OnProxyMaterialLoaded()
{
	DynamicMaterial = UMaterialInstanceDynamic::Create(UNProcGenSettings::Get()->ProxyMaterial.Get(), this);
	Mesh->SetMaterial(0, DynamicMaterial);
	
	DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), NCell->Root.ProxyColor);
	Mesh->MarkRenderStateDirty();
}

void ANCellProxy::Show() const
{
	Mesh->SetVisibility(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ANCellProxy::Hide() const
{
	Mesh->SetVisibility(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
