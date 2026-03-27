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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta=(ToolTip="Should point index 0 be used as the starting point to build the chain, or should we find the left most position?"))
	bool bLeftMostStartingPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta=(ToolTip="Can we assume this is a closed loop?"))
	bool bIsLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta=(ToolTip="Should we calculate a direction to the next point and write it to the attributes?"))
	bool bWriteDirectionAttribute = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName DirectionAttributeName = TEXT("NextDirection");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName TurnAttributeName = TEXT("TurnAngle");
};

class FNSortLineElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
	
};