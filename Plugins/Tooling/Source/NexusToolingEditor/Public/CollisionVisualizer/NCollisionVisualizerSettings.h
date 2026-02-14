// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionVisualizerSettings.generated.h"

UENUM()
enum class ECollisionVisualizerMethod : uint8
{
	LineTrace,
	Sweep,
	Overlap
};

UENUM()
enum class ECollisionVisualizerPrefix : uint8
{
	Single,
	Multi,
	Test
};

UENUM()
enum class ECollisionVisualizerOverlapBlocking : uint8
{
	Blocking,
	Any,
	Multi
};

UENUM()
enum class ECollisionVisualizerBy : uint8
{
	Channel,
	ObjectType,
	Profile
};

UENUM()
enum class ECollisionVisualizerMobility : uint8
{
	Any,
	Static,
	Dynamic
};


UENUM()
enum class ECollisionVisualizerShape : uint8
{
	Box,
	Capsule,
	Sphere
};

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ECollisionVisualizerDrawMode : uint8
{
	None = 0 UMETA(Hidden),
	EditorOnly = 1 << 0 UMETA(DisplayName = "Editor"),
	PlayInEditor =  1 << 1 UMETA(DisplayName = "PIE"),
	SimulateInEditor =  1 << 2 UMETA(DisplayName = "SIE"),
};
ENUM_CLASS_FLAGS(ECollisionVisualizerDrawMode)

USTRUCT()
struct FNCollisionVisualizerPoints
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Start Point", meta=(Tooltip="The ABSOLUTE world location to start the query from."))
	FVector StartPoint = FVector(0,0,0);
	
	UPROPERTY(EditAnywhere, DisplayName="End Point", meta=(Tooltip="The RELATIVE location to the Start Point to query towards."))
	FVector EndPoint = FVector(500,0,0);
	
	UPROPERTY(EditAnywhere, DisplayName="Rotation")
	FRotator Rotation = FRotator(0,0,0);
};

USTRUCT()
struct FNCollisionVisualizerQuery
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Method")
	ECollisionVisualizerMethod QueryMethod = ECollisionVisualizerMethod::LineTrace;
	
	UPROPERTY(EditAnywhere, DisplayName="Type", 
		meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::Overlap", EditConditionHides))
	ECollisionVisualizerPrefix QueryPrefix = ECollisionVisualizerPrefix::Single;
	
	UPROPERTY(EditAnywhere, DisplayName="Blocking", 
		meta=(EditCondition="QueryMethod==ECollisionVisualizerMethod::Overlap", EditConditionHides))
	ECollisionVisualizerOverlapBlocking QueryOverlapBlocking = ECollisionVisualizerOverlapBlocking::Any;
	
	UPROPERTY(EditAnywhere, DisplayName="By")
	ECollisionVisualizerBy QueryBy = ECollisionVisualizerBy::Channel;
	
	UPROPERTY(EditAnywhere, DisplayName="Channel",
		meta=(EditCondition="QueryBy==ECollisionVisualizerBy::Channel", EditConditionHides))
	TEnumAsByte<ECollisionChannel> Channel = ECC_Pawn;
	
	UPROPERTY(EditAnywhere, DisplayName="Object Type",
		meta=(EditCondition="QueryBy==ECollisionVisualizerBy::ObjectType", EditConditionHides))
	TEnumAsByte<EObjectTypeQuery> ObjectType = EObjectTypeQuery::ObjectTypeQuery1;
	
	UPROPERTY(EditAnywhere, DisplayName="Collision Profile",
		meta=(EditCondition="QueryBy==ECollisionVisualizerBy::Profile", EditConditionHides, 
			GetOptions="GetCollisionProfileNames"))
	FName CollisionProfileName = TEXT("BlockAll");
	
	UPROPERTY(EditAnywhere, DisplayName="Shape",
		meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::LineTrace", EditConditionHides))
	ECollisionVisualizerShape QueryShape = ECollisionVisualizerShape::Capsule;
	
	UPROPERTY(EditAnywhere, DisplayName="Radius", 
	meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::LineTrace&&QueryShape==ECollisionVisualizerShape::Capsule", EditConditionHides))
	float ShapeCapsuleRadius = 40.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Half Height",
		meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::LineTrace&&QueryShape==ECollisionVisualizerShape::Capsule", EditConditionHides))
	float ShapeCapsuleHalfHeight = 80.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Half Extent", 
		meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::LineTrace&&QueryShape==ECollisionVisualizerShape::Box", EditConditionHides))
	FVector ShapeBoxHalfExtents = FVector(25,25,25);
	
	UPROPERTY(EditAnywhere, DisplayName="Radius", 
		meta=(EditCondition="QueryMethod!=ECollisionVisualizerMethod::LineTrace&&QueryShape==ECollisionVisualizerShape::Sphere", EditConditionHides))
	float SphereRadius = 42.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Collision Responses",
	meta=(EditCondition="QueryBy==ECollisionVisualizerBy::Channel", EditConditionHides))
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
			using enum ECollisionVisualizerShape;
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
struct FNCollisionVisualizerOptions
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
	ECollisionVisualizerMobility QueryMobility = ECollisionVisualizerMobility::Any;
	
	
	
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
struct FNCollisionVisualizerDrawing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, DisplayName="Draw Mode", meta=(Bitmask,BitmaskEnum="/Script/NexusToolingEditor.ECollisionVisualizerDrawMode"))
	uint8 DrawMode =	static_cast<uint8>(ECollisionVisualizerDrawMode::EditorOnly) | 
						static_cast<uint8>(ECollisionVisualizerDrawMode::PlayInEditor) | 
						static_cast<uint8>(ECollisionVisualizerDrawMode::SimulateInEditor);
	
	UPROPERTY(EditAnywhere, DisplayName="Line Thickness")
	float DrawLineThickness = 1.5f;
	
	UPROPERTY(EditAnywhere, DisplayName="Point Size")
	float DrawPointSize = 15.f;
	
	UPROPERTY(EditAnywhere, DisplayName="Draw Timer", meta=(Tooltip="How long between queries/draws."))
	float DrawTimer = 0.f;
		
	UPROPERTY(EditAnywhere, DisplayName = "Hit Color")
	FColor DrawHitColor = FColor(0,255,88);
	
	UPROPERTY(EditAnywhere, DisplayName = "Mid Color")
	FColor DrawMidColor = FColor(0,0,200);
	
	UPROPERTY(EditAnywhere, DisplayName = "Miss Color")
	FColor DrawMissColor = FColor(255,0,0);
};

USTRUCT()
struct FNCollisionVisualizerSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerPoints Points;
	
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerQuery Query;
	
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerOptions Options;
	
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerDrawing Drawing;
};
