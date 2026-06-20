// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NSortLine2DXYElement.generated.h"

/**
 * PCG settings node that orders a 2D point cloud into a nearest-neighbour chain along XY and
 * optionally annotates every point with turn direction, cardinal facing, and segment metadata.
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNSortLine2DXYSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Sort Line 2D (XY)"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Sorts points into a line using a nearest neighbor chain."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::SortElement; };
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Treat the input as a closed loop, changing how the first and last points are evaluated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Input", meta=(ToolTip="Can we assume this is a closed loop? This effects how the first/last point are evaluated."))
	bool bIsLoop;

	/** When true, start the chain from the left-most point instead of point index 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should point index 0 be used as the starting point to build the chain, or should we find the left most position?"))
	bool bLeftMostStartingPoint;

	/** When true, compute and write the per-point metadata attributes configured below. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Build Additional Metadata?",Category = "Settings|Options", meta=(ToolTip="Should we calculate and write additional metadata information useful for understanding each point on the line?"))
	bool bBuildAdditionalMetadata = true;

	/** When true, rotate each point to face its direction along the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we rotate points to face their direction?", EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	bool bRotatePointToFaceDirection = false;

	/** Attribute name storing each point's grid direction to the next point on the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Next Grid Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's grid direction to the next point on the line."))
	FName NextGridDirectionAttributeName = TEXT("NextGridDirection");
	/** Attribute name storing each point's facing rotation (yaw). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Rotation (Yaw)", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's facing rotation (yaw)."))
	FName FacingRotationAttributeName = TEXT("FacingRotation");
	/** Attribute name storing each point's facing cardinal direction name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Cardinal", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's facing cardinal direction name."))
	FName FacingCardinalAttributeName = TEXT("FacingCardinal");
	/** Attribute name storing each point's facing cardinal direction index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Cardinal Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's facing cardinal direction index."))
	FName FacingCardinalIndexAttributeName = TEXT("FacingCardinalIndex");

	/** Attribute name storing the turn direction taken at each point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Turn Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing the turn direction taken at each point."))
	FName TurnDirectionAttributeName = TEXT("TurnDirection");
	/** Attribute name storing each point's segment index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's segment index."))
	FName SegmentIndexAttributeName = TEXT("SegmentIndex");
	/** Attribute name storing each point's subsegment index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Subsegment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's subsegment index."))
	FName SubsegmentIndexAttributeName = TEXT("SubsegmentIndex");
	/** Attribute name storing the length of the segment each point belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Length", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing the length of the segment each point belongs to."))
	FName SegmentLengthAttributeName = TEXT("SegmentLength");
	/** Attribute name storing each point's classification name (from the Point Names below). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Part Name", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name of the attribute storing each point's classification name (from the Point Names below)."))
	FName PointNameAttributeName = TEXT("PointName");

	/** Point-name value written for points with no special classification. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Default", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for points with no special classification."))
	FName DefaultPointName = TEXT("Default");
	/** Point-name value written for points at a left 90-degree corner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Left 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for points at a left 90-degree corner."))
	FName Left90PointName = TEXT("Left90");
	/** Point-name value written for points at a right 90-degree corner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Right 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for points at a right 90-degree corner."))
	FName Right90PointName = TEXT("Right90");
	/** Point-name value written for points along a straight wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Wall", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for points along a straight wall."))
	FName WallPointName = TEXT("Wall");

	/** Cardinal-name value written for an unknown/undetermined cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Unknown", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for an unknown/undetermined cardinal direction."))
	FName DefaultCardinalName = TEXT("U");
	/** Cardinal-name value written for the North cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="North", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for the North cardinal direction."))
	FName NorthCardinalName = TEXT("N");
	/** Cardinal-name value written for the East cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="East", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for the East cardinal direction."))
	FName EastCardinalName = TEXT("E");
	/** Cardinal-name value written for the South cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="South", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for the South cardinal direction."))
	FName SouthCardinalName = TEXT("S");
	/** Cardinal-name value written for the West cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="West", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides, ToolTip="Name written for the West cardinal direction."))
	FName WestCardinalName = TEXT("W");

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

namespace NEXUS::WorldAssembly::SortLine
{
	/**
	 * Deadzone half-width for turn classification, expressed in sin(theta) space (~30 degrees).
	 * A turn whose magnitude is within this band is treated as a straight wall; anything past it is a
	 * corner, named by the sign of the turn. Axis-aligned grid input yields exactly 0 or +/-1, both well
	 * clear of the band, so the documented grid contract is unaffected while off-grid turns still resolve
	 * to a Left/Right name instead of silently keeping the default classification.
	 */
	inline constexpr float TurnDeadzone = 0.5f;
}

/** Classification of a point along the sorted line, derived from the sign of its turn value. */
enum class ENSortLinePart : uint8
{
	/** A point on a straight run (turn magnitude within the deadzone). */
	Wall,
	/** A left-hand corner (turn past the deadzone, positive winding). */
	Left90,
	/** A right-hand corner (turn past the deadzone, negative winding). */
	Right90
};

/** Per-point line metadata derived purely from turn values; kept PCG-free so it can be unit-tested. */
struct FNSortLinePointInfo
{
	/** How this point is classified along the line. */
	ENSortLinePart Part = ENSortLinePart::Wall;
	/** Index of the segment this point belongs to. */
	int32 SegmentIndex = 0;
	/** Position of this point within its segment. */
	int32 SubsegmentIndex = 0;
	/** Number of points in this point's segment. */
	int32 SegmentLength = 0;
};

/**
 * Executor paired with UNSortLine2DXYSettings.
 */
class FNSortLine2DXYElement : public IPCGElement
{
public:
	/**
	 * Classifies each point of a sorted line from its signed turn value and resolves per-point segment bookkeeping.
	 * Pure and PCG-free so the grid classification and segment maths can be unit-tested directly.
	 * @param TurnValues Signed cross-product magnitude at each point; index 0 is the loop-closure turn (0 for an open line).
	 * @param TurnTolerance Deadzone half-width — a turn whose magnitude is <= TurnTolerance is treated as straight (a wall).
	 * @param OutInfo Receives one entry per input turn value.
	 */
	static NEXUSWORLDASSEMBLY_API void ClassifyLine(const TArray<float>& TurnValues, float TurnTolerance, TArray<FNSortLinePointInfo>& OutInfo);

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
};