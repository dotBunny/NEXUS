// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NSortLineElement.generated.h"

UENUM(BlueprintType)
enum class ENSortLineWindingOrder : uint8
{
	Autodetect = 0 UMETA(DisplayName="Auto-Detect"),
	Clockwise = 1,
	CounterClockwise = 2  UMETA(DisplayName="Counter-Clockwise")
};

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Input", meta=(ToolTip="Should the order of the points be detected or can we make an assumption? This is used when trying to figure out a facing-rotation, for example."))
	ENSortLineWindingOrder WindingOrder = ENSortLineWindingOrder::Autodetect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Input", meta=(ToolTip="Can we assume this is a closed loop?"))
	bool bIsLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should point index 0 be used as the starting point to build the chain, or should we find the left most position?"))
	bool bLeftMostStartingPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we calculate and write additional metadata information?"))
	bool bWriteMetadata = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we write out the segment length to each entry?"))
	bool bWriteSegmentLength = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Options", meta=(ToolTip="Should we calculate and write additional metadata information?"))
	bool bRotatePointToFaceDirection = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName NextGridDirectionAttributeName = TEXT("NextGridDirection");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName FacingRotationAttributeName = TEXT("FacingRotation"); // TODO
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName TurnAttributeName = TEXT("TurnAngle");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName SegmentIndexAttributeName = TEXT("SegmentIndex");  // TODO
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName SubsegmentIndexAttributeName = TEXT("SubsegmentIndex"); // TODO
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata", meta =(EditCondition="bWriteMetadata", EditConditionHides))
	FName SegmentLengthAttributeName = TEXT("SegmentLength");  // TODO
};

class FNSortLineElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
};