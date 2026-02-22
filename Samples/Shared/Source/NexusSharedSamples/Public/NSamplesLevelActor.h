// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSamplesLevelComponents.h"
#include "NSamplesLevelActor.generated.h"

namespace NEXUS::Samples::Level
{
	constexpr float DemoTextSize = 100.f;
	constexpr float WallBaseVerticalOffset = 149.5f;
	constexpr float WallPositionBase = 500.f;
	constexpr float WallPositionOffset = 50.f;
	constexpr float WallScaleBase = 10.f;
	constexpr float WallScaleOffset= 2.f;
}

/**
 * An actor used to control the base NEXUS demo level
 */
UCLASS(DisplayName = "Samples Level Actor", Config = Game, HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, Actor, Input,
	DataLayers, LevelInstance, WorldPartition, HLOD, LOD, Rendering, Collision, Physics, Networking, Replication))
class NEXUSSHAREDSAMPLES_API ANSamplesLevelActor : public AActor
{
	GENERATED_BODY()

public:

	explicit ANSamplesLevelActor(const FObjectInitializer& ObjectInitializer);
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginDestroy() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NEXUS|Space")
	FVector2D AreaSize = FVector2D(4, 7);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NEXUS|Space")
	bool bShowLevelName = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NEXUS|Space")
	bool bShowLogo = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NEXUS|Lighting")
	bool bUseDefaultLighting = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NEXUS|Lighting")
	FRotator SunDirection = FRotator(190.f, 0.f, 0.f);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NEXUS|Cache", DisplayName = "Level Name")
	FText CachedLevelName;
private:

	void ResizeLevel(float InX, float InY) const;
	void SetWallPosition(UStaticMeshComponent* Wall, float Multiplier, bool bNegative, bool bMoveAlongX) const;
	void SetWallScale(UStaticMeshComponent* Wall, float Multiplier, bool bScaleAlongX) const;
	
	const FVector FloorBaseLocation = FVector(0.f,0.f, -0.5f);
	const FVector FloorBaseScale = FVector(1.f,1.f, 4.f);
	
	const FVector WallBasePosition = FVector(0.f,0.f, 149.5f);
	const FVector WallBaseScale = FVector(1.f,1.f, 7.5f);
	
	const FVector BrandBaseScale = FVector(2.f,2.f, 2.f);
	const FRotator BrandBaseRotation = FRotator(-90.f,-90.f, 0.f);
	const FVector BrandBaseLocation = FVector(0.f,0.f, 0.f);

	const FText DemoText = FText::FromString(TEXT("DEMO"));
	
	const FVector DemoNameBaseLocation = FVector(-125.f , 0, 10);
	const FRotator DemoNameBaseRotation = FRotator(90.f, -180.f, 0);
	
	TUniquePtr<FNSamplesLevelActorComponents> Components;
};