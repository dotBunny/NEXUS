#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "NSetPointDirectionPCGElement.generated.h"



// The Settings defines the Node in the Graph
UCLASS(BlueprintType, Blueprintable, ClassGroup = NEXUS)
class UNSetPointDirectionPCGSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Set Points Direction"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Set the points direction to the next point, looping."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif
	
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;

public:
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName OutputAttributeName = TEXT("NextPointRotation");
};

// The Element handles the actual "Work"
class FNSetPointDirectionPCGElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};