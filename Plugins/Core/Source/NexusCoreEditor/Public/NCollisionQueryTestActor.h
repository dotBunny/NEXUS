// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionQueryTestActor.generated.h"

UENUM()
enum class ECollisionQueryTestMethod : uint8
{
	LineTrace,
	Sweep,
	Overlap
};

UENUM()
enum class ECollisionQueryTestPrefix : uint8
{
	Single,
	Multi,
	Test
};

UENUM()
enum class ECollisionQueryTestOverlapBlocking : uint8
{
	Blocking,
	Any,
	Multi
};

UENUM()
enum class ECollisionQueryTestBy : uint8
{
	Channel,
	ObjectType,
	Profile
};

UENUM()
enum class ECollisionQueryTestMobility : uint8
{
	Any,
	Static,
	Dynamic
};


UENUM()
enum class ECollisionQueryTestShape : uint8
{
	Box,
	Capsule,
	Sphere
};

/**
 * Used to test collision queries at design time in the editor.
 * @remarks The was originally started after watching George Prosser's UnrealFest 2023 talk "Collision Data in UE5". 
 * I later found the public version of the tool and was amused at how similar they were. Credits to George and the
 * team at Studio Gobo for the great inspiration, there is just a slightly different spin on the implementation here.
 * @ref https://www.youtube.com/watch?v=xIQI6nXFygA
 * @ref https://github.com/StudioGobo/UECollisionQueryTools 
 */
UCLASS(DisplayName = "NEXUS: Collision Query Test Actor", ClassGroup = "NEXUS", 
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSCOREEDITOR_API ANCollisionQueryTestActor: public AActor
{
	GENERATED_BODY()

	explicit ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer);

public:
	
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Method")
	ECollisionQueryTestMethod QueryMethod = ECollisionQueryTestMethod::LineTrace;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Type", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::Overlap", EditConditionHides))
	ECollisionQueryTestPrefix QueryPrefix = ECollisionQueryTestPrefix::Single;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Blocking", 
		meta=(EditCondition="QueryMethod==ECollisionQueryTestMethod::Overlap", EditConditionHides))
	ECollisionQueryTestOverlapBlocking QueryOverlapBlocking = ECollisionQueryTestOverlapBlocking::Any;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="By")
	ECollisionQueryTestBy QueryBy = ECollisionQueryTestBy::Channel;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Channel",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Channel", EditConditionHides))
	TEnumAsByte<ECollisionChannel> Channel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Object Type",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::ObjectType", EditConditionHides))
	TEnumAsByte<EObjectTypeQuery> ObjectType = EObjectTypeQuery::ObjectTypeQuery1;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Collision Profile",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Profile", EditConditionHides, GetOptions="GetCollisionProfileOptions"))
	FName CollisionProfileName = TEXT("BlockAll");
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Shape",
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace", EditConditionHides))
	ECollisionQueryTestShape QueryShape = ECollisionQueryTestShape::Capsule;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Radius", 
	meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Capsule", EditConditionHides))
	float ShapeCapsuleRadius = 40.f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Half Height",
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Capsule", EditConditionHides))
	float ShapeCapsuleHalfHeight = 80.f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Half Extent", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Box", EditConditionHides))
	FVector ShapeBoxHalfExtents = FVector(25,25,25);
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Radius", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Sphere", EditConditionHides))
	float SphereRadius = 42.f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Query", DisplayName="Collision Responses",
	meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Channel", EditConditionHides))
	FCollisionResponseContainer CollisionResponses;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Trace Complex")
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Find Initial Overlaps")
	bool bFindInitialOverlaps = true;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Ignore Blocks")
	bool bIgnoreBlocks = false;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Ignore Touches")
	bool bIgnoreTouches = false;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Skip Narrow Phase")
	bool bSkipNarrowPhase = false;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Mobility Type")
	ECollisionQueryTestMobility QueryMobility = ECollisionQueryTestMobility::Any;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Options", DisplayName="Update Timer", meta=(Tooltip="How long between queries."))
	float UpdateTimer = 0.f;
	

	UPROPERTY(EditAnywhere, Category = "Collision Test|Drawing", DisplayName="Line Thickness")
	float DrawLineThickness = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Drawing", DisplayName="Point Size")
	float DrawPointSize = 15.f;
	
protected:
	UFUNCTION()
	static TArray<FName> GetCollisionProfileNames();
	
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USceneComponent> StartPointComponent;
	
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USceneComponent> EndPointComponent;

private:
	void Query(float DrawTime) const;
	
	void DoLineTraceSingle(float DrawTime) const;
	void DoLineTraceMulti(float DrawTime) const;
	void DoLineTraceTest(float DrawTime) const;
	
	void DoSweepSingle(float DrawTime) const;
	void DoSweepMulti(float DrawTime) const;
	void DoSweepTest(float DrawTime) const;
	
	void DoOverlapBlocking(float DrawTime) const;
	void DoOverlapAny(float DrawTime) const;
	void DoOverlapMulti(float DrawTime) const;
	
	FCollisionQueryParams  GetCollisionQueryParams() const;
	FCollisionResponseParams GetCollisionResponseParams() const;
	FCollisionShape GetCollisionShape() const;
	
	static EQueryMobilityType ToQueryMobilityType(const ECollisionQueryTestMobility Mobility)
	{
		switch (Mobility)
		{
			using enum ECollisionQueryTestMobility;
			case Any: 
				return EQueryMobilityType::Any;
			case Static: 
				return EQueryMobilityType::Static;
			case Dynamic: 
				return EQueryMobilityType::Dynamic;
			default: 
				return EQueryMobilityType::Any;
		}
	}
	
	float TickTimer = 0;
};