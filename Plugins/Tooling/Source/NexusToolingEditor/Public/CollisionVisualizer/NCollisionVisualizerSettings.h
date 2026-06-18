// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionVisualizerSettings.generated.h"

/**
 * Which UWorld query flavor the collision visualizer should issue.
 */
UENUM()
enum class ENCollisionVisualizerMethod : uint8
{
	LineTrace,
	Sweep,
	Overlap
};

/**
 * Trace/sweep result mode (single result, multi result, or boolean test).
 */
UENUM()
enum class ENCollisionVisualizerPrefix : uint8
{
	Single,
	Multi,
	Test
};

/**
 * Overlap flavor — blocking-only, any-overlap, or multi-overlap.
 */
UENUM()
enum class ENCollisionVisualizerOverlapBlocking : uint8
{
	Blocking,
	Any,
	Multi
};

/**
 * Which UE collision query dispatch (channel / object type / named profile) to use.
 */
UENUM()
enum class ENCollisionVisualizerBy : uint8
{
	Channel,
	ObjectType,
	Profile
};

/**
 * Mobility filter applied to the query — maps onto EQueryMobilityType.
 */
UENUM()
enum class ENCollisionVisualizerMobility : uint8
{
	Any,
	Static,
	Dynamic
};


/**
 * Primitive shape used by sweep/overlap queries.
 */
UENUM()
enum class ENCollisionVisualizerShape : uint8
{
	Box,
	Capsule,
	Sphere
};

/**
 * Bitflag mask selecting which editor contexts the visualizer should render into.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENCollisionVisualizerDrawMode : uint8
{
	None = 0 UMETA(Hidden),
	EditorOnly = 1  UMETA(DisplayName = "Editor"),
	PlayInEditor =  2 UMETA(DisplayName = "PIE"),
	SimulateInEditor =  4 UMETA(DisplayName = "SIE"),
};
ENUM_CLASS_FLAGS(ENCollisionVisualizerDrawMode)

/**
 * Start/end positions and orientation used to drive a single collision-visualizer query.
 */
USTRUCT()
struct FNCollisionVisualizerPoints
{
	GENERATED_BODY()
	
	/** Absolute world location the query starts from. */
	UPROPERTY(EditAnywhere, DisplayName="Start Point", meta=(Tooltip="The ABSOLUTE world location to start the query from."))
	FVector StartPoint = FVector(0,0,0);

	/** End location relative to StartPoint that the query traces towards. */
	UPROPERTY(EditAnywhere, DisplayName="End Point", meta=(Tooltip="The RELATIVE location to the Start Point to query towards."))
	FVector EndPoint = FVector(500,0,0);

	/** World-space rotation applied to the query shape. */
	UPROPERTY(EditAnywhere, DisplayName="Rotation", meta=(Tooltip="World-space rotation applied to the query shape."))
	FRotator Rotation = FRotator(0,0,0);
};

/**
 * Query parameters (method, channel/profile/type, shape) that describe what to trace for.
 */
USTRUCT()
struct FNCollisionVisualizerQuery
{
	GENERATED_BODY()
	
	/** Kind of collision query to perform (line trace, sweep, or overlap). */
	UPROPERTY(EditAnywhere, DisplayName="Method", meta=(Tooltip="The kind of collision query to perform (line trace, sweep, or overlap)."))
	ENCollisionVisualizerMethod QueryMethod = ENCollisionVisualizerMethod::LineTrace;

	/** For trace/sweep methods: return a single hit, multiple hits, or a boolean test. */
	UPROPERTY(EditAnywhere, DisplayName="Type",
		meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::Overlap", EditConditionHides, Tooltip="Whether the query returns a single hit or multiple hits."))
	ENCollisionVisualizerPrefix QueryPrefix = ENCollisionVisualizerPrefix::Single;

	/** For the overlap method: which overlap responses count as hits (blocking, any, multi). */
	UPROPERTY(EditAnywhere, DisplayName="Blocking",
		meta=(EditCondition="QueryMethod==ENCollisionVisualizerMethod::Overlap", EditConditionHides, Tooltip="Which overlap responses are treated as hits."))
	ENCollisionVisualizerOverlapBlocking QueryOverlapBlocking = ENCollisionVisualizerOverlapBlocking::Any;

	/** How collision is filtered: by trace channel, object type, or collision profile. */
	UPROPERTY(EditAnywhere, DisplayName="By", meta=(Tooltip="How collision is filtered: by trace channel, object type, or collision profile."))
	ENCollisionVisualizerBy QueryBy = ENCollisionVisualizerBy::Channel;

	/** Trace channel used when querying by channel. */
	UPROPERTY(EditAnywhere, DisplayName="Channel",
		meta=(EditCondition="QueryBy==ENCollisionVisualizerBy::Channel", EditConditionHides, Tooltip="Trace channel used when querying by channel."))
	TEnumAsByte<ECollisionChannel> Channel = ECC_Pawn;

	/** Object type used when querying by object type. */
	UPROPERTY(EditAnywhere, DisplayName="Object Type",
		meta=(EditCondition="QueryBy==ENCollisionVisualizerBy::ObjectType", EditConditionHides, Tooltip="Object type used when querying by object type."))
	TEnumAsByte<EObjectTypeQuery> ObjectType = EObjectTypeQuery::ObjectTypeQuery1;

	/** Collision profile used when querying by profile. */
	UPROPERTY(EditAnywhere, DisplayName="Collision Profile",
		meta=(EditCondition="QueryBy==ENCollisionVisualizerBy::Profile", EditConditionHides,
			GetOptions="GetCollisionProfileNames", Tooltip="Collision profile used when querying by profile."))
	FName CollisionProfileName = TEXT("BlockAll");

	/** Primitive shape swept through the world for non-line-trace queries. */
	UPROPERTY(EditAnywhere, DisplayName="Shape",
		meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::LineTrace", EditConditionHides, Tooltip="Shape swept through the world for non-line-trace queries."))
	ENCollisionVisualizerShape QueryShape = ENCollisionVisualizerShape::Capsule;

	/** Radius of the capsule query shape. */
	UPROPERTY(EditAnywhere, DisplayName="Radius",
	meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::LineTrace&&QueryShape==ENCollisionVisualizerShape::Capsule", EditConditionHides, Tooltip="Radius of the capsule query shape."))
	float ShapeCapsuleRadius = 40.f;

	/** Half-height of the capsule query shape. */
	UPROPERTY(EditAnywhere, DisplayName="Half Height",
		meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::LineTrace&&QueryShape==ENCollisionVisualizerShape::Capsule", EditConditionHides, Tooltip="Half-height of the capsule query shape."))
	float ShapeCapsuleHalfHeight = 80.f;

	/** Half-extents of the box query shape. */
	UPROPERTY(EditAnywhere, DisplayName="Half Extent",
		meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::LineTrace&&QueryShape==ENCollisionVisualizerShape::Box", EditConditionHides, Tooltip="Half-extents of the box query shape."))
	FVector ShapeBoxHalfExtents = FVector(25,25,25);

	/** Radius of the sphere query shape. */
	UPROPERTY(EditAnywhere, DisplayName="Radius",
		meta=(EditCondition="QueryMethod!=ENCollisionVisualizerMethod::LineTrace&&QueryShape==ENCollisionVisualizerShape::Sphere", EditConditionHides, Tooltip="Radius of the sphere query shape."))
	float SphereRadius = 42.f;

	/** Per-channel collision responses applied when querying by channel. */
	UPROPERTY(EditAnywhere, DisplayName="Collision Responses",
	meta=(EditCondition="QueryBy==ENCollisionVisualizerBy::Channel", EditConditionHides, Tooltip="Per-channel collision responses applied when querying by channel."))
	FCollisionResponseContainer CollisionResponses;
	
	/** @return An FCollisionResponseParams populated from CollisionResponses for use with channel-based queries. */
	FCollisionResponseParams GetCollisionResponseParams() const
	{
		FCollisionResponseParams Parameters;
		Parameters.CollisionResponse = CollisionResponses;
		return Parameters;
	}

	/**
	 * Builds the FCollisionShape implied by QueryShape plus its associated dimensions.
	 * @return A Box/Capsule/Sphere FCollisionShape; default-constructed if QueryShape is unrecognized.
	 */
	FCollisionShape GetCollisionShape() const
	{
		switch (QueryShape)
		{
			using enum ENCollisionVisualizerShape;
		case Box:
			return FCollisionShape::MakeBox(ShapeBoxHalfExtents);
		case Capsule:
			return FCollisionShape::MakeCapsule(ShapeCapsuleRadius, ShapeCapsuleHalfHeight);
		case Sphere:
			return FCollisionShape::MakeSphere(SphereRadius);
		}
		return FCollisionShape();
	}

	/**
	 * Supplies the options list for the CollisionProfileName picker metadata.
	 * @return Names of every collision profile registered with UCollisionProfile.
	 */
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

		return Names;
	}
};

/**
 * Secondary FCollisionQueryParams toggles (complex, narrow-phase, mobility).
 */
USTRUCT()
struct FNCollisionVisualizerOptions
{
	GENERATED_BODY()
	
	/** Trace against complex (per-poly) collision instead of simple collision. */
	UPROPERTY(EditAnywhere, DisplayName="Trace Complex", meta=(Tooltip="Trace against complex (per-poly) collision instead of simple collision."))
	bool bTraceComplex = false;

	/** Report overlaps already present at the sweep's start location. */
	UPROPERTY(EditAnywhere, DisplayName="Find Initial Overlaps", meta=(Tooltip="Report overlaps already present at the sweep's start location."))
	bool bFindInitialOverlaps = true;

	/** Exclude blocking hits from the results. */
	UPROPERTY(EditAnywhere, DisplayName="Ignore Blocks", meta=(Tooltip="Exclude blocking hits from the results."))
	bool bIgnoreBlocks = false;

	/** Exclude touch/overlap hits from the results. */
	UPROPERTY(EditAnywhere, DisplayName="Ignore Touches", meta=(Tooltip="Exclude touch/overlap hits from the results."))
	bool bIgnoreTouches = false;

	/** Skip the narrow phase, returning only broad-phase results. */
	UPROPERTY(EditAnywhere, DisplayName="Skip Narrow Phase", meta=(Tooltip="Skip the narrow phase, returning only broad-phase results."))
	bool bSkipNarrowPhase = false;

	/** Restrict results to actors of the given mobility. */
	UPROPERTY(EditAnywhere, DisplayName="Mobility Type", meta=(Tooltip="Restrict results to actors of the given mobility."))
	ENCollisionVisualizerMobility QueryMobility = ENCollisionVisualizerMobility::Any;
	
	
	
	/** @return A FCollisionQueryParams populated from the boolean toggles and mobility filter on this struct. */
	FCollisionQueryParams GetCollisionQueryParams() const
	{
		FCollisionQueryParams Parameters;
		Parameters.bTraceComplex = bTraceComplex;
		Parameters.bFindInitialOverlaps = bFindInitialOverlaps;
		Parameters.bIgnoreBlocks = bIgnoreBlocks;
		Parameters.bIgnoreTouches = bIgnoreTouches;
		Parameters.bSkipNarrowPhase = bSkipNarrowPhase;
		Parameters.MobilityType = ToQueryMobilityType(QueryMobility);
		return Parameters;
	}

	/** Map the visualizer's mobility enum onto the engine's EQueryMobilityType. */
	static EQueryMobilityType ToQueryMobilityType(const ENCollisionVisualizerMobility Mobility)
	{
		switch (Mobility)
		{
			using enum ENCollisionVisualizerMobility;
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
};

/**
 * Rendering controls for visualizer output — draw mode mask, thickness, colors, and cadence.
 */
USTRUCT()
struct FNCollisionVisualizerDrawing
{
	GENERATED_BODY()

	/** Bitmask of editor contexts the visualization is drawn in (Editor, Play-In-Editor, Simulate-In-Editor). */
	UPROPERTY(EditAnywhere, DisplayName="Draw Mode", meta=(Bitmask,BitmaskEnum="/Script/NexusToolingEditor.ENCollisionVisualizerDrawMode", Tooltip="Contexts in which the visualization is drawn (Editor, Play-In-Editor, Simulate-In-Editor)."))
	uint8 DrawMode =	static_cast<uint8>(ENCollisionVisualizerDrawMode::EditorOnly) |
						static_cast<uint8>(ENCollisionVisualizerDrawMode::PlayInEditor) |
						static_cast<uint8>(ENCollisionVisualizerDrawMode::SimulateInEditor);

	/** Thickness of the drawn query lines. */
	UPROPERTY(EditAnywhere, DisplayName="Line Thickness", meta=(Tooltip="Thickness of the drawn query lines."))
	float DrawLineThickness = 1.5f;

	/** Size of the drawn impact points. */
	UPROPERTY(EditAnywhere, DisplayName="Point Size", meta=(Tooltip="Size of the drawn impact points."))
	float DrawPointSize = 15.f;

	/** Seconds between successive queries/draws (0 = every tick). */
	UPROPERTY(EditAnywhere, DisplayName="Draw Timer", meta=(Tooltip="How long between queries/draws."))
	float DrawTimer = 0.f;

	/** Color used to draw blocking hits (the line up to the impact and the impact points). */
	UPROPERTY(EditAnywhere, DisplayName = "Hit Color", meta=(Tooltip="Color used to draw blocking hits (the line up to the impact and the impact points)."))
	FColor DrawHitColor = FColor(0,255,88);

	/** Color used to draw non-blocking touches and overlaps. */
	UPROPERTY(EditAnywhere, DisplayName = "Mid Color", meta=(Tooltip="Color used to draw non-blocking touches and overlaps."))
	FColor DrawMidColor = FColor(0,0,200);

	/** Color used to draw queries that hit nothing. */
	UPROPERTY(EditAnywhere, DisplayName = "Miss Color", meta=(Tooltip="Color used to draw queries that hit nothing."))
	FColor DrawMissColor = FColor(255,0,0);
};

/**
 * Full settings bundle persisted by the collision-visualizer widget between sessions.
 */
USTRUCT()
struct FNCollisionVisualizerSettings
{
	GENERATED_BODY()

	/** Start/end positions and orientation used to drive the visualizer query. */
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerPoints Points;

	/** Query configuration (method, channel/profile/type, shape dimensions). */
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerQuery Query;

	/** Secondary FCollisionQueryParams toggles applied alongside the main query. */
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerOptions Options;

	/** Rendering controls — draw mode mask, thickness, colors, and cadence. */
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerDrawing Drawing;
};
