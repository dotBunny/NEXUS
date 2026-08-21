// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "Templates/Function.h"

#include "NFilterEdgePoints2DXYElement.generated.h"

/**
 * Parameters for UNFilterEdgePoints2DXYSettings.
 */
USTRUCT(BlueprintType)
struct FNFilterEdgePoints2DParams
{
	GENERATED_BODY()

	/** Grid spacing (world units) used when testing for neighbours to identify border points. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ClampMin = "0.0", PCG_Overridable, ToolTip = "Grid spacing in world units, used to size the neighbour search. Match it to the spacing of the incoming points; too small and every point reads as an edge, too large and the border dissolves."))
	float Spacing = 100.f;

	/** How many rings of edge points to take, measured in points rather than world units. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ClampMin = "1", PCG_Overridable, ToolTip = "How many rings of edge points to take, measured in points rather than world units. One is the outline alone; higher values re-run the edge test on what is left and take that ring too. Every ring arrives together on OutsideFilter."))
	int32 Depth = 1;

	/** When true, write the ring each point was taken on out as an attribute. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, DisplayName = "Write Edge Depth?", Category = Settings, meta = (PCG_Overridable, ToolTip = "Should the ring each point was taken on be written out as an attribute? Rings number from one at the outline inwards, and the interior points on InsideFilter get zero."))
	bool bWriteEdgeDepth = false;

	/** Attribute name storing the ring each point was taken on. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, DisplayName = "Edge Depth", Category = Settings, meta = (EditCondition = "bWriteEdgeDepth", EditConditionHides, PCG_Overridable, ToolTip = "Name of the attribute storing the ring each point was taken on."))
	FName EdgeDepthAttributeName = TEXT("EdgeDepth");
};

/**
 * PCG settings node that splits a filled 2D grid into its interior and its border, classifying points by
 * XY neighbour count within Spacing * 1.5 (>= 8 neighbours is interior).
 *
 * Depth widens the border beyond the outline: each ring past the first is what the same test finds once the
 * previous ring has been taken away, so a Depth of three returns a band three points thick.
 *
 * @remark The border points are emitted on the OutsideFilter pin; InsideFilter carries the interior fill.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/pcg/elements/filter-edge-points-2d-xy/">UNFilterEdgePoints2DXYSettings</a>
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNFilterEdgePoints2DXYSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Filter Edge Points 2D (XY)"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Finds and identifies edge (border) points of a filled set of 2D points along the XY axis."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::FilterElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Instance of the per-evaluation parameter struct, exposed as PCG-overridable. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNFilterEdgePoints2DParams FilterParams;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNFilterEdgePoints2DXYSettings.
 */
class FNFilterEdgePoints2DXYElement : public IPCGElement
{
public:
	/**
	 * Peels rings of edge points off a filled point set, labelling each point with the ring it came off on.
	 *
	 * Taking a ring lowers the live neighbour tally of every point it touched, so each round reads the counts a
	 * fresh pass over the remaining points would have produced — the same answer as re-running the edge test on
	 * the leftovers, without walking the whole set again. Neighbour lookup is left to the caller, which keeps
	 * the peel free of PCG types so it can be unit-tested directly.
	 *
	 * @param NumPoints How many points are in the set.
	 * @param MaxDepth How many rings to take; values below one are treated as one.
	 * @param VisitNeighbors Reports the neighbours of the point at the supplied index, excluding the point itself. Runs on several threads during the initial count, so it must be safe to call concurrently.
	 * @param OutDepths Receives one ring index per point, in input order: one for the outermost ring rising inward, zero for the interior points that survived the peel.
	 */
	static NEXUSCORE_API void AssignEdgeDepths(int32 NumPoints, int32 MaxDepth, TFunctionRef<void(int32, TFunctionRef<void(int32)>)> VisitNeighbors, TArray<int32>& OutDepths);

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }

};
