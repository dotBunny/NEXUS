// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NSortLineElement.generated.h"


UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNSortLineSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Sort Line"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Sorts points into a line using a nearest neighbor chain."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::SortElement; };
	//virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	virtual FPCGElementPtr CreateElement() const override;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Input", meta=(ToolTip="Can we assume this is a closed loop?"))
	bool bIsLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should point index 0 be used as the starting point to build the chain, or should we find the left most position?"))
	bool bLeftMostStartingPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Calculate Additional Metadata?",Category = "Settings|Options", meta=(ToolTip="Should we calculate and write additional metadata information?"))
	bool bWriteAdditionalMetadata = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we rotate points to face their direction?", EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	bool bRotatePointToFaceDirection = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Next Grid Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName NextGridDirectionAttributeName = TEXT("NextGridDirection");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Facing Rotation", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName FacingRotationAttributeName = TEXT("FacingRotation"); // TODO
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Turn Direction", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName TurnDirectionAttributeName = TEXT("TurnDirection");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName SegmentIndexAttributeName = TEXT("SegmentIndex");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Subsegment Index", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName SubsegmentIndexAttributeName = TEXT("SubsegmentIndex");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Segment Length", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName SegmentLengthAttributeName = TEXT("SegmentLength");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Part Name", Category = "Settings|Metadata (Attribute Names)", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName PointNameAttributeName = TEXT("PointName");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Default", Category = "Settings|Point Names", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName DefaultPointName = TEXT("Default");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Corner 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName Left90PointName = TEXT("Left90");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Corner 90-Deg", Category = "Settings|Point Names", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName Right90PointName = TEXT("Right90");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Wall", Category = "Settings|Point Names", meta=(EditCondition="bWriteAdditionalMetadata", EditConditionHides))
	FName WallPointName = TEXT("Wall");

};

class FNSortLineElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
};