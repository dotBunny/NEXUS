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
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"), false);
	SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = SceneRoot;
	
	// Create Floor
	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Floor"), false);
	Floor->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FloorMeshAsset(TEXT("/Engine/MapTemplates/SM_Template_Map_Floor"));
	if (FloorMeshAsset.Succeeded())
	{
		Floor->SetStaticMesh(FloorMeshAsset.Object);
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FloorMaterialAsset(TEXT("/Engine/OpenWorldTemplate/LandscapeMaterial/MI_ProcGrid"));
		if (FloorMaterialAsset.Succeeded())
		{
			Floor->SetMaterial(0, FloorMaterialAsset.Object);
		}
	}

	// Floor - Position & Scale
	Floor->SetWorldLocation(FloorBaseLocation);
	Floor->SetWorldScale3D(FloorBaseScale);

	// Create Decal
	Brand = CreateDefaultSubobject<UDecalComponent>(TEXT("D_Brand"), false);
	Brand->SetupAttachment(RootComponent);
	Brand->SetWorldLocation(BrandBaseLocation);
	Brand->SetWorldScale3D(BrandBaseScale);
	Brand->SetWorldRotation(BrandBaseRotation);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BrandMaterialAsset(TEXT("/NexusSharedSamples/M_NSamples_Decal"));
	if (BrandMaterialAsset.Succeeded())
	{
		Brand->SetMaterial(0, BrandMaterialAsset.Object);
	}
	
	// Create Demo Name
	DemoName = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TR_DemoName"), false);
	DemoName->SetupAttachment(RootComponent);
	DemoName->SetWorldLocation(DemoNameBaseLocation);
	DemoName->SetWorldRotation(DemoNameBaseRotation);
	DemoName->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
	DemoName->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	DemoName->WorldSize = DemoTextSize;
	DemoName->SetText(DemoText);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DemoNameMaterialAsset(TEXT("/NexusMaterialLibrary/Debug/MI_NDebugText_Blue"));
	if (DemoNameMaterialAsset.Succeeded())
	{
		DemoName->SetMaterial(0, DemoNameMaterialAsset.Object);
	}
	DemoName->SetCastShadow(true);
	DemoName->SetReceivesDecals(false);
	
	// Create Walls
	WallNorth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_North"), false);
	WallNorth->SetupAttachment(RootComponent);
	WallEast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_East"), false);
	WallEast->SetupAttachment(RootComponent);
	WallSouth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_South"), false);
	WallSouth->SetupAttachment(RootComponent);
	WallWest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Wall_West"), false);
	WallWest->SetupAttachment(RootComponent);	
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WallMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (WallMeshAsset.Succeeded())
	{
		WallNorth->SetStaticMesh(WallMeshAsset.Object);
		WallEast->SetStaticMesh(WallMeshAsset.Object);
		WallSouth->SetStaticMesh(WallMeshAsset.Object);
		WallWest->SetStaticMesh(WallMeshAsset.Object);
		
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> WallMaterialAsset(TEXT("/NexusMaterialLibrary/Debug/MI_NDebug_Grey"));
		if (WallMaterialAsset.Succeeded())
		{
			WallNorth->SetMaterial(0, WallMaterialAsset.Object);
			WallEast->SetMaterial(0, WallMaterialAsset.Object);
			WallSouth->SetMaterial(0, WallMaterialAsset.Object);
			WallWest->SetMaterial(0, WallMaterialAsset.Object);
		}
	}

	// Walls - Position & Scale
	WallNorth->SetWorldLocation(WallBasePosition);
	WallNorth->SetWorldScale3D(WallBaseScale);
	WallEast->SetWorldLocation(WallBasePosition);
	WallEast->SetWorldScale3D(WallBaseScale);
	WallSouth->SetWorldLocation(WallBasePosition);
	WallSouth->SetWorldScale3D(WallBaseScale);
	WallWest->SetWorldLocation(WallBasePosition);
	WallWest->SetWorldScale3D(WallBaseScale);

	Floor->SetMobility(EComponentMobility::Static);
	WallNorth->SetMobility(EComponentMobility::Static);
	WallEast->SetMobility(EComponentMobility::Static);
	WallSouth->SetMobility(EComponentMobility::Static);
	WallWest->SetMobility(EComponentMobility::Static);
	DemoName->SetMobility(EComponentMobility::Static);
	Brand->SetMobility(EComponentMobility::Static);

	// Lighting
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"), false);
	DirectionalLight->SetupAttachment(RootComponent);
	DirectionalLight->SetWorldLocation(FVector(0.f, 0.f, 400.f));
	
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"), false);
	ExponentialHeightFog->SetupAttachment(RootComponent);
	ExponentialHeightFog->SetWorldLocation(FVector(0.f, 0.f, -6850.f));
	
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"), false);
	SkyAtmosphere->SetupAttachment(RootComponent);
	SkyAtmosphere->SetWorldLocation(FVector(0.f, 0.f, -6000.f));

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"), false);
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetWorldLocation(FVector(0.f, 0.f, 600.f));

	SkySphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphere"), false);
	SkySphere->SetupAttachment(RootComponent);
	SkySphere->SetWorldScale3D(FVector(400.f, 400.f, 400.f));
	SkySphere->SetWorldLocation(FVector(0.f, 70.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkySphereMeshAsset(TEXT("/Engine/EngineSky/SM_SkySphere"));
	if (SkySphereMeshAsset.Succeeded())
	{
		SkySphere->SetStaticMesh(SkySphereMeshAsset.Object);
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkySphereMaterialAsset(TEXT("/Engine/EngineSky/M_SimpleSkyDome"));
		if (SkySphereMaterialAsset.Succeeded())
		{
			SkySphere->SetMaterial(0, SkySphereMaterialAsset.Object);
		}
	}
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"), false);
	VolumetricCloud->SetupAttachment(RootComponent);
	VolumetricCloud->SetWorldLocation(FVector(0.f,0.f, 700.f));
}

void ANSamplesLevelActor::OnConstruction(const FTransform& Transform)
{
	// Rebuild Level Geometry and general stuff
	ResizeLevel(AreaSize.X, AreaSize.Y);

	// Handle lighting options
	DirectionalLight->SetVisibility(bUseDefaultLighting);
	DirectionalLight->SetWorldRotation(SunDirection);
	ExponentialHeightFog->SetVisibility(bUseDefaultLighting);
	SkyAtmosphere->SetVisibility(bUseDefaultLighting);
	SkyLight->SetVisibility(bUseDefaultLighting);
	SkySphere->SetVisibility(bUseDefaultLighting);
	VolumetricCloud->SetVisibility(bUseDefaultLighting);

	// Update with level name
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Replace(TEXT("DEMO_"), TEXT(""));
	CachedLevelName = FText::AsLocalizable_Advanced(
		TEXT("NSamplesLevelActor"),
		LevelName,
		LevelName);

	if (DemoName != nullptr)
	{
		DemoName->SetText(CachedLevelName);
	}
	
	Super::OnConstruction(Transform);
}

void ANSamplesLevelActor::ResizeLevel(const float InX, const float InY) const
{
	// Get transform information
	FVector RootLocation = RootComponent->GetComponentLocation();
	FRotator RootRotation = RootComponent->GetComponentRotation();
	FVector RootScale = RootComponent->GetComponentScale();

	// Change floor
	if (Floor != nullptr)
	{
		FVector FloorScale = Floor->GetComponentScale();
		FloorScale.X = InX;
		FloorScale.Y = InY;
		Floor->SetWorldScale3D(FloorScale);
	}

	if (WallNorth != nullptr)
	{
		SetWallPosition(WallNorth, InX, false, true);
		SetWallScale(WallNorth, InY, false);
	}

	if (WallSouth != nullptr)
	{
		SetWallPosition(WallSouth, InX, true, true);
		SetWallScale(WallSouth, InY, false);
	}

	if (WallEast != nullptr)
	{
		SetWallPosition(WallEast, InY, false, false);
		SetWallScale(WallEast, InX, true);
	}

	if (WallWest != nullptr)
	{
		SetWallPosition(WallWest, InY, true, false);
		SetWallScale(WallWest, InX, true);
	}

	if (Brand != nullptr)
	{
		Brand->SetWorldLocation(RootLocation + BrandBaseLocation);
		Brand->SetWorldRotation(BrandBaseRotation);
		Brand->SetWorldScale3D(BrandBaseScale);
	}

	if (DemoName != nullptr)
	{
		DemoName->SetWorldLocation(RootLocation + DemoNameBaseLocation);
		DemoName->SetWorldRotation(DemoNameBaseRotation);
		DemoName->SetWorldScale3D(FVector::OneVector);
	}
}

void ANSamplesLevelActor::SetWallPosition(UStaticMeshComponent* Wall, const float Multiplier, const bool bNegative, const bool bMoveAlongX) const
{
	float Position =  ((WallPositionBase * Multiplier) + WallPositionOffset);
	if (bNegative) Position *= -1.f;

	FVector WorkingLocation = RootComponent->GetComponentLocation();
	if (bMoveAlongX)
	{
		WorkingLocation.X += Position;
		//WorkingLocation.Y = 0.f;
	}
	else
	{
		//WorkingLocation.X = WorkingLocation.Y;
		WorkingLocation.Y += Position;
		//WorkingLocation.X = 0.f;
	}
	WorkingLocation.Z += WallBaseVerticalOffset;

	Wall->SetWorldLocation(WorkingLocation);
}

void ANSamplesLevelActor::SetWallScale(UStaticMeshComponent* Wall, const float Multiplier, const bool bScaleAlongX) const
{
	FVector Scale = WallBaseScale;
	if (bScaleAlongX)
	{
		Scale.X = (WallScaleBase * Multiplier) + WallScaleOffset;
	}
	else
	{
		Scale.Y = (WallScaleBase * Multiplier);
	}
	Wall->SetWorldScale3D(Scale);
}
