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
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Input", meta=(ToolTip="Can we assume this is a closed loop? This effects how the first/last point are evaluated."))
	bool bIsLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should point index 0 be used as the starting point to build the chain, or should we find the left most position?"))
	bool bLeftMostStartingPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Build Additional Metadata?",Category = "Settings|Options", meta=(ToolTip="Should we calculate and write additional metadata information useful for understanding each point on the line?"))
	bool bBuildAdditionalMetadata = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we rotate points to face their direction?", EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	bool bRotatePointToFaceDirection = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Next Grid Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName NextGridDirectionAttributeName = TEXT("NextGridDirection");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Rotation (Yaw)", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName FacingRotationAttributeName = TEXT("FacingRotation");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Cardinal", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName FacingCardinalAttributeName = TEXT("FacingCardinal");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Cardinal", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName FacingCardinalIndexAttributeName = TEXT("FacingCardinalIndex");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Turn Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName TurnDirectionAttributeName = TEXT("TurnDirection");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName SegmentIndexAttributeName = TEXT("SegmentIndex");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Subsegment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName SubsegmentIndexAttributeName = TEXT("SubsegmentIndex");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Length", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName SegmentLengthAttributeName = TEXT("SegmentLength");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Part Name", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName PointNameAttributeName = TEXT("PointName");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Default", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName DefaultPointName = TEXT("Default");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Corner 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName Left90PointName = TEXT("Left90");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Corner 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName Right90PointName = TEXT("Right90");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Wall", Category = "Settings|Point Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName WallPointName = TEXT("Wall");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Unknown", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName DefaultCardinalName = TEXT("U");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="North", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName NorthCardinalName = TEXT("N");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="East", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName EastCardinalName = TEXT("E");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="South", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName SouthCardinalName = TEXT("S");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="West", Category = "Settings|Cardinal Names", meta=(EditCondition="bBuildAdditionalMetadata", EditConditionHides))
	FName WestCardinalName = TEXT("W");

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNSortLine2DXYSettings.
 */
class FNSortLine2DXYElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
};