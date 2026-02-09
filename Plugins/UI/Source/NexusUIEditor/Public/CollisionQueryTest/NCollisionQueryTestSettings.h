// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionQueryTestSettings.generated.h"

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


USTRUCT()
struct FNCollisionQueryTestPoints
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Start Point (Absolute)")
	FVector StartPoint = FVector(0,0,0);
	
	UPROPERTY(EditAnywhere, DisplayName="End Point (Relative)")
	FVector EndPoint = FVector(0,0,500);
	
	UPROPERTY(EditAnywhere, DisplayName="Rotation")
	FRotator Rotation = FRotator(0,0,0);
};

USTRUCT()
struct FNCollisionQueryTestQuery
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Method")
	ECollisionQueryTestMethod QueryMethod = ECollisionQueryTestMethod::LineTrace;
	
	UPROPERTY(EditAnywhere, DisplayName="Type", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::Overlap", EditConditionHides))
	ECollisionQueryTestPrefix QueryPrefix = ECollisionQueryTestPrefix::Single;
	
	UPROPERTY(EditAnywhere, DisplayName="Blocking", 
		meta=(EditCondition="QueryMethod==ECollisionQueryTestMethod::Overlap", EditConditionHides))
	ECollisionQueryTestOverlapBlocking QueryOverlapBlocking = ECollisionQueryTestOverlapBlocking::Any;
	
	UPROPERTY(EditAnywhere, DisplayName="By")
	ECollisionQueryTestBy QueryBy = ECollisionQueryTestBy::Channel;
	
	UPROPERTY(EditAnywhere, DisplayName="Channel",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Channel", EditConditionHides))
	TEnumAsByte<ECollisionChannel> Channel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, DisplayName="Object Type",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::ObjectType", EditConditionHides))
	TEnumAsByte<EObjectTypeQuery> ObjectType = EObjectTypeQuery::ObjectTypeQuery1;
	
	UPROPERTY(EditAnywhere, DisplayName="Collision Profile",
		meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Profile", EditConditionHides, 
			GetOptions="GetCollisionProfileNames"))
	FName CollisionProfileName = TEXT("BlockAll");
	
	UPROPERTY(EditAnywhere, DisplayName="Shape",
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace", EditConditionHides))
	ECollisionQueryTestShape QueryShape = ECollisionQueryTestShape::Capsule;
	
	UPROPERTY(EditAnywhere, DisplayName="Radius", 
	meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Capsule", EditConditionHides))
	float ShapeCapsuleRadius = 40.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Half Height",
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Capsule", EditConditionHides))
	float ShapeCapsuleHalfHeight = 80.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Half Extent", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Box", EditConditionHides))
	FVector ShapeBoxHalfExtents = FVector(25,25,25);
	
	UPROPERTY(EditAnywhere, DisplayName="Radius", 
		meta=(EditCondition="QueryMethod!=ECollisionQueryTestMethod::LineTrace&&QueryShape==ECollisionQueryTestShape::Sphere", EditConditionHides))
	float SphereRadius = 42.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Collision Responses",
	meta=(EditCondition="QueryBy==ECollisionQueryTestBy::Channel", EditConditionHides))
	FCollisionResponseContainer CollisionResponses;
	
	FCollisionResponseParams GetCollisionResponseParams() const
	{
		FCollisionResponseParams Parameters;
		Parameters.CollisionResponse = CollisionResponses;
		return MoveTemp(Parameters);
	}
	
	FCollisionShape GetCollisionShape() const
	{
		switch (QueryShape)
		{
			using enum ECollisionQueryTestShape;
		case Box:
			return FCollisionShape::MakeBox(ShapeBoxHalfExtents);
		case Capsule:
			return FCollisionShape::MakeCapsule(ShapeCapsuleRadius, ShapeCapsuleHalfHeight);
		case Sphere:
			return FCollisionShape::MakeSphere(SphereRadius);
		}
		return FCollisionShape();
	}
	
	TArray<FName> GetCollisionProfileNames() const
	{
		TArray<FName> Names;
		const UCollisionProfile* CollisionProfile = UCollisionProfile::Get();
		const int32 Count = CollisionProfile->GetNumOfProfiles();
		Names.Reserve(Count);

		for (int32 i = 0; i < Count; ++i)
		{
			const FCollisionResponseTemplate* ProfileTemplate = CollisionProfile->GetProfileByIndex(i);
			Names.Add(ProfileTemplate->Name);
		}

		return MoveTemp(Names);
	}
};

USTRUCT()
struct FNCollisionQueryTestOptions
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Trace Complex")
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, DisplayName="Find Initial Overlaps")
	bool bFindInitialOverlaps = true;
	
	UPROPERTY(EditAnywhere, DisplayName="Ignore Blocks")
	bool bIgnoreBlocks = false;
	
	UPROPERTY(EditAnywhere, DisplayName="Ignore Touches")
	bool bIgnoreTouches = false;
	
	UPROPERTY(EditAnywhere, DisplayName="Skip Narrow Phase")
	bool bSkipNarrowPhase = false;
	
	UPROPERTY(EditAnywhere, DisplayName="Mobility Type")
	ECollisionQueryTestMobility QueryMobility = ECollisionQueryTestMobility::Any;
	
	UPROPERTY(EditAnywhere, DisplayName="Update Timer", meta=(Tooltip="How long between queries."))
	float UpdateTimer = 0.f;
	
	FCollisionQueryParams GetCollisionQueryParams() const
	{
		FCollisionQueryParams Parameters;
		Parameters.bTraceComplex = bTraceComplex;
		Parameters.bFindInitialOverlaps = bFindInitialOverlaps;
		Parameters.bIgnoreBlocks = bIgnoreBlocks;
		Parameters.bIgnoreTouches = bIgnoreTouches;
		Parameters.bSkipNarrowPhase = bSkipNarrowPhase;
		return MoveTemp(Parameters);
	}
};

USTRUCT()
struct FNCollisionQueryTestDrawing
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Drawing", DisplayName="Line Thickness")
	float DrawLineThickness = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Drawing", DisplayName="Point Size")
	float DrawPointSize = 15.f;
	
	UPROPERTY(EditAnywhere, Category = "Collision Test|Drawing", DisplayName="Labels")
	bool DrawLabels = false;
};


USTRUCT()
struct FNCollisionQueryTestSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FNCollisionQueryTestPoints Points;
	
	UPROPERTY(EditAnywhere)
	FNCollisionQueryTestQuery Query;
	
	UPROPERTY(EditAnywhere)
	FNCollisionQueryTestOptions Options;
	
	UPROPERTY(EditAnywhere)
	FNCollisionQueryTestDrawing Drawing;
};
