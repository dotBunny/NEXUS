// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NFilterBoundsElement.generated.h"


UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNFilterBoundsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

	const FName SourceAttribute = FName("Sources");
	const FName TargetAttribute = FName("Targets");
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Filter Bounds"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Filters points based on their bounds and their collision with target bounds."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::FilterElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	virtual FPCGElementPtr CreateElement() const override;
};

class FNFilterBoundsElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
	
};