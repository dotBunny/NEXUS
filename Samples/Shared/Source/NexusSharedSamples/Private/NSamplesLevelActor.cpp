// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesLevelActor.h"

#include "Components/DecalComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"

ANSamplesLevelActor::ANSamplesLevelActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Components = MakeUnique<FNSamplesLevelActorComponents>();
	
	Components->SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"), false);
	Components->SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = Components->SceneRoot;
	
	// Create Floor
	Components->Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Floor"), false);
	Components->Floor->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FloorMeshAsset(TEXT("/Engine/MapTemplates/SM_Template_Map_Floor"));
	if (FloorMeshAsset.Succeeded())
	{
		Components->Floor->SetStaticMesh(FloorMeshAsset.Object);
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FloorMaterialAsset(TEXT("/Engine/OpenWorldTemplate/LandscapeMaterial/MI_ProcGrid"));
		if (FloorMaterialAsset.Succeeded())
		{
			Components->Floor->SetMaterial(0, FloorMaterialAsset.Object);
		}
	}

	// Floor - Position & Scale
	Components->Floor->SetWorldLocation(FloorBaseLocation);
	Components->Floor->SetWorldScale3D(FloorBaseScale);

	// Create Decal
	Components->Brand = CreateDefaultSubobject<UDecalComponent>(TEXT("D_Brand"), false);
	Components->Brand->SetupAttachment(RootComponent);
	Components->Brand->SetWorldLocation(BrandBaseLocation);
	Components->Brand->SetWorldScale3D(BrandBaseScale);
	Components->Brand->SetWorldRotation(BrandBaseRotation);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BrandMaterialAsset(TEXT("/NexusSharedSamples/M_NSamples_Decal"));
	if (BrandMaterialAsset.Succeeded())
	{
		Components->Brand->SetMaterial(0, BrandMaterialAsset.Object);
	}
	
	// Create Demo Name
	Components->DemoName = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TR_DemoName"), false);
	Components->DemoName->SetupAttachment(RootComponent);
	Components->DemoName->SetWorldLocation(DemoNameBaseLocation);
	Components->DemoName->SetWorldRotation(DemoNameBaseRotation);
	Components->DemoName->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
	Components->DemoName->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	Components->DemoName->WorldSize = NEXUS::Samples::Level::DemoTextSize;
	Components->DemoName->SetText(DemoText);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DemoNameMaterialAsset(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/DebugText/MI_NDebugText_White"));
	if (DemoNameMaterialAsset.Succeeded())
	{
		Components->DemoName->SetMaterial(0, DemoNameMaterialAsset.Object);
	}
	Components->DemoName->SetCastShadow(true);
	Components->DemoName->SetReceivesDecals(false);
	
	// Create Walls
	Components->WallNorth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_North"), false);
	Components->WallNorth->SetupAttachment(RootComponent);
	Components->WallEast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_East"), false);
	Components->WallEast->SetupAttachment(RootComponent);
	Components->WallSouth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_South"), false);
	Components->WallSouth->SetupAttachment(RootComponent);
	Components->WallWest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_West"), false);
	Components->WallWest->SetupAttachment(RootComponent);	
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WallMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (WallMeshAsset.Succeeded())
	{
		Components->WallNorth->SetStaticMesh(WallMeshAsset.Object);
		Components->WallEast->SetStaticMesh(WallMeshAsset.Object);
		Components->WallSouth->SetStaticMesh(WallMeshAsset.Object);
		Components->WallWest->SetStaticMesh(WallMeshAsset.Object);
		
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> WallMaterialAsset(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_Grey"));
		if (WallMaterialAsset.Succeeded())
		{
			Components->WallNorth->SetMaterial(0, WallMaterialAsset.Object);
			Components->WallEast->SetMaterial(0, WallMaterialAsset.Object);
			Components->WallSouth->SetMaterial(0, WallMaterialAsset.Object);
			Components->WallWest->SetMaterial(0, WallMaterialAsset.Object);
		}
	}

	// Walls - Position & Scale
	Components->WallNorth->SetWorldLocation(WallBasePosition);
	Components->WallNorth->SetWorldScale3D(WallBaseScale);
	Components->WallEast->SetWorldLocation(WallBasePosition);
	Components->WallEast->SetWorldScale3D(WallBaseScale);
	Components->WallSouth->SetWorldLocation(WallBasePosition);
	Components->WallSouth->SetWorldScale3D(WallBaseScale);
	Components->WallWest->SetWorldLocation(WallBasePosition);
	Components->WallWest->SetWorldScale3D(WallBaseScale);

	Components->Floor->SetMobility(EComponentMobility::Static);
	Components->WallNorth->SetMobility(EComponentMobility::Static);
	Components->WallEast->SetMobility(EComponentMobility::Static);
	Components->WallSouth->SetMobility(EComponentMobility::Static);
	Components->WallWest->SetMobility(EComponentMobility::Static);
	Components->DemoName->SetMobility(EComponentMobility::Static);
	Components->Brand->SetMobility(EComponentMobility::Static);

	// Lighting
	Components->DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"), false);
	Components->DirectionalLight->SetupAttachment(RootComponent);
	Components->DirectionalLight->SetWorldLocation(FVector(0.f, 0.f, 400.f));
	
	Components->ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"), false);
	Components->ExponentialHeightFog->SetupAttachment(RootComponent);
	Components->ExponentialHeightFog->SetWorldLocation(FVector(0.f, 0.f, -6850.f));
	
	Components->SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"), false);
	Components->SkyAtmosphere->SetupAttachment(RootComponent);
	Components->SkyAtmosphere->SetWorldLocation(FVector(0.f, 0.f, -6000.f));

	Components->SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"), false);
	Components->SkyLight->SetupAttachment(RootComponent);
	Components->SkyLight->SetWorldLocation(FVector(0.f, 0.f, 600.f));

	Components->SkySphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphere"), false);
	Components->SkySphere->SetupAttachment(RootComponent);
	Components->SkySphere->SetWorldScale3D(FVector(400.f, 400.f, 400.f));
	Components->SkySphere->SetWorldLocation(FVector(0.f, 70.f, 0.f));
	

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkySphereMeshAsset(TEXT("/Engine/EngineSky/SM_SkySphere"));
	if (SkySphereMeshAsset.Succeeded())
	{
		Components->SkySphere->SetStaticMesh(SkySphereMeshAsset.Object);
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkySphereMaterialAsset(TEXT("/Engine/EngineSky/M_SimpleSkyDome"));
		if (SkySphereMaterialAsset.Succeeded())
		{
			Components->SkySphere->SetMaterial(0, SkySphereMaterialAsset.Object);
		}
		Components->SkySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Components->VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"), false);
	Components->VolumetricCloud->SetupAttachment(RootComponent);
	Components->VolumetricCloud->SetWorldLocation(FVector(0.f,0.f, 700.f));
}

void ANSamplesLevelActor::OnConstruction(const FTransform& Transform)
{
	// Rebuild Level Geometry and general stuff
	ResizeLevel(AreaSize.X, AreaSize.Y);

	// Handle lighting options
	Components->DirectionalLight->SetVisibility(bUseDefaultLighting);
	Components->DirectionalLight->SetWorldRotation(SunDirection);
	Components->ExponentialHeightFog->SetVisibility(bUseDefaultLighting);
	Components->SkyAtmosphere->SetVisibility(bUseDefaultLighting);
	Components->SkyLight->SetVisibility(bUseDefaultLighting);
	Components->SkySphere->SetVisibility(bUseDefaultLighting);
	Components->VolumetricCloud->SetVisibility(bUseDefaultLighting);

	// Update with level name
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Replace(TEXT("DEMO_"), TEXT(""));
	CachedLevelName = FText::AsLocalizable_Advanced(
		TEXT("NSamplesLevelActor"),
		LevelName,
		LevelName);

	if (Components->DemoName != nullptr)
	{
		Components->DemoName->SetText(CachedLevelName);
	}
	
	Components->Brand->SetVisibility(bShowLogo);
	Components->DemoName->SetVisibility(bShowLevelName);
	
	Super::OnConstruction(Transform);
}

void ANSamplesLevelActor::BeginDestroy()
{
	if (Components != nullptr)
	{
		Components.Reset();
	}
	Super::BeginDestroy();
}

void ANSamplesLevelActor::ResizeLevel(const float InX, const float InY) const
{
	// Get transform information
	const FVector RootLocation = RootComponent->GetComponentLocation();

	// Change floor
	if (Components->Floor != nullptr)
	{
		FVector FloorScale = Components->Floor->GetComponentScale();
		FloorScale.X = InX;
		FloorScale.Y = InY;
		Components->Floor->SetWorldScale3D(FloorScale);
	}

	if (Components->WallNorth != nullptr)
	{
		SetWallPosition(Components->WallNorth, InX, false, true);
		SetWallScale(Components->WallNorth, InY, false);
	}

	if (Components->WallSouth != nullptr)
	{
		SetWallPosition(Components->WallSouth, InX, true, true);
		SetWallScale(Components->WallSouth, InY, false);
	}

	if (Components->WallEast != nullptr)
	{
		SetWallPosition(Components->WallEast, InY, false, false);
		SetWallScale(Components->WallEast, InX, true);
	}

	if (Components->WallWest != nullptr)
	{
		SetWallPosition(Components->WallWest, InY, true, false);
		SetWallScale(Components->WallWest, InX, true);
	}

	if (Components->Brand != nullptr)
	{
		Components->Brand->SetWorldLocation(RootLocation + BrandBaseLocation);
		Components->Brand->SetWorldRotation(BrandBaseRotation);
		Components->Brand->SetWorldScale3D(BrandBaseScale);
	}

	if (Components->DemoName != nullptr)
	{
		Components->DemoName->SetWorldLocation(RootLocation + DemoNameBaseLocation);
		Components->DemoName->SetWorldRotation(DemoNameBaseRotation);
		Components->DemoName->SetWorldScale3D(FVector::OneVector);
	}
}

void ANSamplesLevelActor::SetWallPosition(UStaticMeshComponent* Wall, const float Multiplier, const bool bNegative, const bool bMoveAlongX) const
{
	float Position =  ((NEXUS::Samples::Level::WallPositionBase * Multiplier) + NEXUS::Samples::Level::WallPositionOffset);
	if (bNegative) Position *= -1.f;

	FVector WorkingLocation = RootComponent->GetComponentLocation();
	if (bMoveAlongX)
	{
		WorkingLocation.X += Position;
	}
	else
	{
		WorkingLocation.Y += Position;
	}
	WorkingLocation.Z += NEXUS::Samples::Level::WallBaseVerticalOffset;

	Wall->SetWorldLocation(WorkingLocation);
}

void ANSamplesLevelActor::SetWallScale(UStaticMeshComponent* Wall, const float Multiplier, const bool bScaleAlongX) const
{
	FVector Scale = WallBaseScale;
	if (bScaleAlongX)
	{
		Scale.X = (NEXUS::Samples::Level::WallScaleBase * Multiplier) + NEXUS::Samples::Level::WallScaleOffset;
	}
	else
	{
		Scale.Y = (NEXUS::Samples::Level::WallScaleBase * Multiplier);
	}
	Wall->SetWorldScale3D(Scale);
}
