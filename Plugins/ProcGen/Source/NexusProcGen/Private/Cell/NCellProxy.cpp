// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellProxy.h"

#include "Cell/NCell.h"
#include "Cell/NCellLevelInstance.h"
#include "NColor.h"
#include "Components/BillboardComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "LevelInstance/LevelInstanceActor.h"

ANCellProxy::ANCellProxy(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// Add our physics shape
	Mesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->Mobility = EComponentMobility::Movable;

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterial> ProxyMaterial;
		FName ID_Info;
		FText NAME_Info;
		FConstructorStatics()
			: ProxyMaterial(TEXT("/NexusProcGen/M_NCellProxy"))
			, ID_Info(TEXT("Info"))
			, NAME_Info(NSLOCTEXT("MaterialCategory", "Info", "Info"))
		{}
	};
	
	// Mesh Visible-stuff
	Mesh->SetEnableRaytracing(false);
	Mesh->SetVisibleInRayTracing(false);
	DynamicMaterial = UMaterialInstanceDynamic::Create(FConstructorStatics().ProxyMaterial.Get(), this);
	Mesh->SetMaterial(0, DynamicMaterial);
	Mesh->bExplicitShowWireframe = true;
	Mesh->WireframeColor = FLinearColor::Gray;
	Mesh->SetTangentsType(EDynamicMeshComponentTangentsMode::AutoCalculated);
	
	// Mesh Collisions
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->EnableComplexAsSimpleCollision();
	
#if WITH_EDITOR	
	bCanPlayFromHere = 0;
#endif
	
	N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT("/NexusProcGen/EditorResources/S_NCellProxy", RootComponent, false, 0.5f)
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

	const FVector SpawnLocation = this->GetActorLocation();
	const FRotator SpawnRotation = this->GetActorRotation();

	AActor* SpawnedLevelInstanceActor = GetWorld()->SpawnActor(
		ANCellLevelInstance::StaticClass(), &SpawnLocation, &SpawnRotation, SpawnInfo);

	LevelInstance = Cast<ANCellLevelInstance>(SpawnedLevelInstanceActor);
	
	// TODO: I suspect this might need work as the actual level instance has a CookedWorldAsset
	LevelInstance->SetWorldAsset(NCell->World);

#if WITH_EDITOR
	LevelInstance->SetActorLabel(FString::Printf(TEXT("%s_LevelInstance"), *this->GetActorLabel()), false);
#endif
}

void ANCellProxy::LoadLevelInstance()
{
	if (LevelInstance == nullptr)
	{
		CreateLevelInstance();
	}
	if (!LevelInstance->IsLoaded())
	{
		LevelInstance->LoadLevelInstance();

		// TODO: Hook up Actor? / Root Component / Flag as being sapwned?
		
		Hide();
	}
}

void ANCellProxy::UnloadLevelInstance()
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
		LevelInstance->Destroy();
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
#endif	

	// Convert our mesh data to UE
	Mesh->SetMesh(NCell->Root.Hull.CreateDynamicMesh(true));
	
	// Setup collision
	if (UBodySetup* BodySetup = Mesh->GetBodySetup())
	{
		BodySetup->Modify();
	}
	
	FKConvexElem ConvexHull;
	ConvexHull.VertexData = NCell->Root.Hull.Vertices;
	ConvexHull.IndexData = NCell->Root.Hull.GetFlatIndices();
	ConvexHull.CalcAABB(FTransform::Identity, FVector::One());

	FKAggregateGeom AggGeom;;
	AggGeom.ConvexElems.Add(ConvexHull);
	
	Mesh->SetSimpleCollisionShapes(AggGeom, true);		

	// Lets rock some colors
	Mesh->WireframeColor = NCell->Root.ProxyColor;
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
